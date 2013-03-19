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

#include <stddef.h>
#include <stdint.h>

#define S3P_MAX_SIZE 256
#define S3P_OVERHEAD 3

typedef enum {
  S3P_SUCCESS,
  S3P_BUF_TOO_SMALL, // Output buffer too small
  S3P_CHECKSUM_ERR, // Received and calculated checksums don't match
  S3P_PARSE_FAILURE, // The packet does not follow protocol specification
} S3P_ERR;

typedef enum  {
  S3P_START = 0x5B,  // Marks the start of a packet. ASCII '['
  S3P_TERM = 0x5D,   // Marks the end of a packet, ASCII ']'
  S3P_ESCAPE = 0x5E, // Marks escaped bytes, ASCII '^'
  S3P_MASK = 0x20,   // Mask used for escaping chars. echar = char ^ S3P_MASK
} S3P_CONTROL;

/**
   Build a new packet from the bytes in "data" into "out".

   S3P packets can encapsulate arbitrary data, but "out" must be at least
   S3P_OVERHEAD bytes longer than the data that needs to be packetized; and this
   can increase to twofold depending on the number of bytes that need
   escaping. In general, unless memory usage is a concern, allocate twice as
   much space as the data you wish to packetize. Keep in mind that the output
   buffer is used for intermediate steps, and may contain junk data if the
   function exits with an error.

   Params:
   in: a byte array of data to be packetized.
   in_size: the length of "in".
   out: a byte array which will hold the built packet.
   out_size: the length of "out".
   packet_size: a pointer to an int which will contain the length of the
   built packet.
*/
S3P_ERR s3p_build(uint8_t const *in, size_t in_size, uint8_t *out, size_t out_size, 
                    size_t *packet_size);

/**
   Read a packet from "in", and place unescaped data into "data".
    
   Breaking streams into packets is the responsibility of the caller. This
   function will only read the first packet in the "in" buffer; all other data
   will be ignored. This function guarantees that the length of the data
   retrieved from the packet will be strictly less than the size of the input
   packet. Keep in mind that the output buffer is used for intermediate decoding
   steps and may contain junk data if the function exits unsuccessfully.

   Params:
   in: byte array of raw, packetized data.
   in_size: the length of "in".
   out: byte array which will contain the retrieved data.
   out_size: the length of "out".
   packet_size: pointer to an int which will contain the length of the
   retrieved data.
*/
S3P_ERR s3p_read(uint8_t const *in, size_t in_size, uint8_t *out, size_t out_size, 
                   size_t *packet_size);

#endif
