/**
   libS3P - Super Simple Streaming Protocol
   Written by Nick Pascucci
   08.26.2012
 */
#ifndef S3P_H
#define S3P_H

#include <stdint.h>

#define S3P_OVERHEAD 3

typedef enum {
  S3P_SUCCESS = 0,
  S3P_BUF_TOO_SMALL = 1,
  S3P_PAYLOAD_TOO_LARGE = 2,
  S3P_CHECKSUM_ERR = 3,
  S3P_PARSE_FAILURE = 4,
} S3P_ERR;

/**
   Build a new packet from the bytes in "data" into "out".

   S3P packets are limited to 256 bytes of data; attempting to packetize more
   will result in a S3P_PAYLOAD_TOO_LARGE error. "out" must be at least
   S3P_OVERHEAD bytes longer than the data that needs to be packetized; and this
   can increase to twofold depending on the number of bytes that need
   escaping. In general, unless memory usage is a concern, allocate twice as
   much space as the data you wish to packetize.

   Params:
   data: a byte array of data to be packetized.
   dsize: the length of "data".
   out: a byte array which will hold the built packet.
   osize: the length of "out".
   psize: a pointer to an int which will contain the length of the
   built packet.
*/
S3P_ERR s3p_build(uint8_t const *data, int dsize, uint8_t *out, int osize, 
                    int *psize);

/**
   Read a packet from "in", and place unescaped data into "data".
    
   Breaking streams into packets is the responsibility of the 
   caller. This function will only read the first packet in the
   "in" buffer; all other data will be ignored. This function guarantees that
   the length of the data retrieved from the packet will be strictly less than
   the size of the input packet.
      
   Params:
   in: byte array of raw, packetized data.
   isize: the length of "in".
   data: byte array which will contain the retrieved data.
   dsize: the length of "data".
   psize: pointer to an int which will contain the length of the
   retrieved data.
*/
S3P_ERR s3p_read(uint8_t const *in, int isize, uint8_t *data, int dsize, 
                   int *psize);

#endif
