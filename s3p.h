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
