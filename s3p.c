#include <s3p.h>

enum S3P_CONTROL_CHARS {
  S3P_START = 0x56,  // Marks the start of a packet
  S3P_ESCAPE = 0x25, // Marks escaped bytes
  S3P_MASK = 0x20,   // Mask used for escaping chars. echar = char ^ S3P_MASK
};

S3P_ERR s3p_build(uint8_t const *data, int dsize, uint8_t *out, int osize, 
                  int *psize){
  if((dsize + S3P_OVERHEAD) > osize){
    return S3P_BUF_TOO_SMALL;
  }
  if(255 < dsize){
    return S3P_PAYLOAD_TOO_LARGE;
  }

  /*
    S3P packets follow this structure:
    START | LENGTH (N) | DATA 1 | DATA 2 | ... | DATA N | CHECKSUM
   */

  out[0] = S3P_START;
  
  int dnext = 2; // Next data byte position in output buffer

  /*
    If the length of the data (dsize, in this case) is S3P_START or S3P_ESCAPE
    we need to escape it.
   */
  if(S3P_START == dsize || S3P_ESCAPE == dsize){
    out[1] = S3P_ESCAPE;
    out[2] = ((uint8_t) dsize) ^ S3P_MASK;
    dnext = 3;
  } else {
    out[1] = (uint8_t) dsize;
  }

  uint8_t check = 0; // Checksum
  int i;
  for(i=0; i<dsize; i++){
    // Check the size restrictions: dnext should contain the number of bytes
    // written so far less one (as the indices start at zero), and we need to
    // add a checksum byte after all is said and done.
    if(dnext + 2 > osize){
      return S3P_BUF_TOO_SMALL;
    }

    uint8_t dbyte = data[i];
    check += dbyte;

    if(S3P_START == dbyte || S3P_ESCAPE == dbyte){
      out[dnext] = S3P_ESCAPE;
      dnext++;
      out[dnext] = dbyte ^ S3P_MASK;
    } else {
      out[dnext] = dbyte;
    }
    dnext++;
  }

  out[dnext] = check;
  dnext++;
  *psize = dnext;
  return S3P_SUCCESS;
}

S3P_ERR s3p_read(uint8_t const *in, int isize, uint8_t *data, int dsize, 
                 int *psize){
  if(dsize < 1){
    return S3P_BUF_TOO_SMALL;
  }
  if(isize < S3P_OVERHEAD){
    return S3P_PARSE_FAILURE;
  }
  // The first byte should be a start byte. If not, this is not an S3P packet
  // and we should not try to parse it.
  if(S3P_START != in[0]){
    return S3P_PARSE_FAILURE;
  }
  
  int dnext = 2;
  int length;
  if(S3P_ESCAPE == in[1]){
    length = in[2] ^ S3P_MASK;
    dnext++;
  } else {
    length = in[1];
  }

  int dread = 0; // Number of data bytes read
  uint8_t check = 0; // Checksum
  while(dread < length){
    if(dread >= dsize){
      return S3P_BUF_TOO_SMALL;
    }
    if(dnext >= isize){
      return S3P_PARSE_FAILURE;
    }
    
    uint8_t dbyte = in[dnext];
    if(S3P_START == dbyte){
      return S3P_PARSE_FAILURE;
    }
    if(S3P_ESCAPE == dbyte){
      dnext++;
      dbyte = in[dnext];
      if(S3P_START == dbyte || S3P_ESCAPE == dbyte){
        return S3P_PARSE_FAILURE;
      }
      dbyte = dbyte ^ S3P_MASK;
    }

    data[dread] = dbyte;
    check += dbyte;
    dnext++;
    dread++;
  }
  *psize = dread;
  
  uint8_t pcheck;
  if(S3P_ESCAPE == in[dnext]){
    dnext++;
    pcheck = in[dnext] ^ S3P_MASK;
  } else {
    pcheck = in[dnext];
  }

  if(pcheck != check){
    return S3P_CHECKSUM_ERR;
  }

  return S3P_SUCCESS;
}
