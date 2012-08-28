/**
   libS3P: A Super Simple Streaming Protocol implementation.
   Copyright (C) 2012 Nicholas Pascucci (npascut1@gmail.com)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "s3p.h"

enum S3P_CONTROL_CHARS {
  S3P_START = 0x56,  // Marks the start of a packet
  S3P_ESCAPE = 0x25, // Marks escaped bytes
  S3P_MASK = 0x20,   // Mask used for escaping chars. echar = char ^ S3P_MASK
};

S3P_ERR s3p_build(uint8_t const *data, size_t dsize, uint8_t *out, size_t osize, 
                  size_t *psize){
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
  
  int data_next = 2;

  if(S3P_START == dsize || S3P_ESCAPE == dsize){
    out[1] = S3P_ESCAPE;
    out[2] = ((uint8_t) dsize) ^ S3P_MASK;
    data_next = 3;
  } else {
    out[1] = (uint8_t) dsize;
  }

  uint8_t checksum = 0;
  int i;
  for(i=0; i<dsize; i++){
    // Check the output buffer size restrictions: data_next should contain the
    // number of bytes written so far less one (as the indices start at zero),
    // and we need to add a checksum byte after all is said and done.
    if(data_next + 2 > osize){
      return S3P_BUF_TOO_SMALL;
    }

    uint8_t dbyte = data[i];
    checksum += dbyte;

    if(S3P_START == dbyte || S3P_ESCAPE == dbyte){
      out[data_next] = S3P_ESCAPE;
      data_next++;
      out[data_next] = dbyte ^ S3P_MASK;
    } else {
      out[data_next] = dbyte;
    }
    data_next++;
  }

  out[data_next] = checksum;
  data_next++;
  *psize = data_next;
  return S3P_SUCCESS;
}

S3P_ERR s3p_read(uint8_t const *in, size_t isize, uint8_t *data, 
                 size_t dsize, size_t *psize){
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
  uint8_t checksum = 0;
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
    checksum += dbyte;
    dnext++;
    dread++;
  }
  *psize = dread;
  
  uint8_t pchecksum;
  if(S3P_ESCAPE == in[dnext]){
    dnext++;
    pchecksum = in[dnext] ^ S3P_MASK;
  } else {
    pchecksum = in[dnext];
  }

  if(pchecksum != checksum){
    return S3P_CHECKSUM_ERR;
  }

  return S3P_SUCCESS;
}
