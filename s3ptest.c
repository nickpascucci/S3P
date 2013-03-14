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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <s3p.h>

#define NORM   "\x1B[0m"
#define RED    "\x1B[31m"
#define GREEN  "\x1B[32m"

#define CHECK(COND) check(COND, __LINE__)
#define COLOR(TEXT, COL) printf("%s%s%s", COL, TEXT, NORM)

void check(int condition, int line);
uint8_t get_checksum(uint8_t *buf, size_t length);
void test_s3p_build_no_escape();
void test_s3p_build_with_escape();
void test_s3p_read_no_escape();
void test_s3p_read_with_escape();
void test_s3p_read_without_start_byte();
void test_s3p_read_with_mixed_packet();
void test_s3p_read_bad_checksum();
void test_s3p_read_truncated_packet();
void test_s3p_read_empty_packet();
void test_s3p_build_with_small_buf();
void test_s3p_read_oversize_packet();
void test_s3p_build_read_integration();

int main(void){  printf("Super Simple Streaming Protocol Test Program\n");

  test_s3p_build_no_escape();
  test_s3p_build_with_escape();
  test_s3p_build_with_small_buf();
  test_s3p_read_no_escape();
  test_s3p_read_with_escape();
  test_s3p_read_without_start_byte();
  test_s3p_read_with_mixed_packet();
  test_s3p_read_bad_checksum();
  test_s3p_read_truncated_packet();
  test_s3p_read_empty_packet();
  test_s3p_read_oversize_packet();
  test_s3p_build_read_integration();
  
  COLOR("ALL PASS\n", GREEN);
  return 0;
}

void check(int condition, int line){
  if(!condition){
    COLOR("FAIL", RED);
    printf(" @%d\n", line);
    exit(1);
  }
}

void test_s3p_build_no_escape(){
  printf("Testing s3p_build(), no escaping. ");
  uint8_t data[] = { 0x00, 0x01, 0x02, 0x03 };
  uint8_t out[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_build(data, sizeof data, out, 20, &psize);
  CHECK(S3P_SUCCESS == err);
  CHECK(7 == psize);
  uint8_t template[] = { S3P_START, 0x00, 0x01, 0x02, 0x03, 0x06, S3P_TERM };
  for(size_t i=0; i<psize; i++){
    CHECK(out[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_with_escape(){
  printf("Testing s3p_build(), with escaping. ");
  uint8_t data[] = { S3P_ESCAPE, 0x01, S3P_START, 0x03 };
  uint8_t checksum = get_checksum(data, sizeof data);
  uint8_t out[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_build(data, sizeof data, out, 20, &psize);
  CHECK(S3P_SUCCESS == err);
  CHECK(9 == psize);
  uint8_t template[] = { S3P_START, S3P_ESCAPE, S3P_ESCAPE ^ S3P_MASK, 0x01, S3P_ESCAPE, S3P_START ^ S3P_MASK, 0x03, checksum, S3P_TERM };
  for(size_t i=0; i<psize; i++){
    CHECK(out[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_with_small_buf(){
  printf("Testing s3p_build(), with small buffer. ");
  uint8_t data[] = { S3P_ESCAPE, 0x01, S3P_START, 0x03 };
  uint8_t out[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_build(data, sizeof data, out, 1, &psize);
  CHECK(S3P_BUF_TOO_SMALL == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_no_escape(){
  printf("Testing s3p_read(), no escaping. ");
  uint8_t in[] = { S3P_START, 0x00, 0x01, 0x02, 0x03, 0x06, S3P_TERM };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);

  CHECK(S3P_SUCCESS == err);
  CHECK(4 == psize);
  
  for(size_t i=0; i<psize; i++){
    CHECK(data[i] == i);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_with_escape(){
  printf("Testing s3p_read(), with escaping. ");
  uint8_t in[] = { S3P_START, S3P_ESCAPE, S3P_ESCAPE ^ S3P_MASK, 0x01, S3P_ESCAPE, S3P_START ^ S3P_MASK, 0x03, 0x00, S3P_TERM };
  uint8_t template[] = { S3P_ESCAPE, 0x01, S3P_START, 0x03 };
  in[7] = get_checksum(template, sizeof template);
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);

  CHECK(S3P_SUCCESS == err);
  CHECK(4 == psize);

  for(size_t i=0; i<psize; i++){
    CHECK(data[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_with_mixed_packet(){
  printf("Testing s3p_read(), with mixed packet. ");
  uint8_t in[] = { S3P_START, S3P_ESCAPE, S3P_START, 0x01, S3P_ESCAPE, 0x76, 0x03, 0x7F, S3P_TERM };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);  
}

void test_s3p_read_without_start_byte(){
  printf("Testing s3p_read(), with no start byte. ");
  uint8_t in[] = { 0x00, S3P_ESCAPE, S3P_START, 0x01, S3P_ESCAPE, 0x76, 0x03, 0x7F, S3P_TERM };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_bad_checksum(){
  printf("Testing s3p_read(), with bad checksum. ");
  uint8_t in[] = { S3P_START, 0x01, 0x76, 0x03, 0x70, S3P_TERM };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_CHECKSUM_ERR == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_truncated_packet(){
  printf("Testing s3p_read(), with truncated packet. ");
  uint8_t in[] = { S3P_START, 0x01, 0x76, 0x03, 0x70 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_empty_packet(){
  printf("Testing s3p_read(), with empty packet. ");
  uint8_t in[5] = { 0x00 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, 0, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_oversize_packet(){
  printf("Testing s3p_read(), with oversized packet. ");
  uint8_t in[] = { S3P_START, 0x01, 0x76, 0x03, 0x70, S3P_TERM };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  size_t psize;
  err = s3p_read(in, sizeof in, data, 1, &psize);
  CHECK(S3P_BUF_TOO_SMALL == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_read_integration(){
  printf("Testing s3p_build() and s3p_read() integration. ");
  uint8_t in_data[] = { S3P_ESCAPE, 0x01, S3P_START, 0x03, S3P_TERM };
  uint8_t built_packet[20] = { 0x00 };
  
  S3P_ERR err;
  size_t psize;
  err = s3p_build(in_data, sizeof in_data, built_packet, sizeof built_packet, 
                  &psize);
  
  CHECK(S3P_SUCCESS == err);
  CHECK(11 == psize);
  
  size_t built_size = psize;
  uint8_t out_data[20] = { 0x00 };
  err = s3p_read(built_packet, built_size, out_data, sizeof out_data, &psize);

  CHECK(S3P_SUCCESS == err);
  CHECK(sizeof in_data == psize);
  
  for(size_t i=0; i<psize; i++){
    CHECK(in_data[i] == out_data[i]);
  }

  COLOR("Pass\n", GREEN);
}

uint8_t get_checksum(uint8_t *buf, size_t length){
  uint8_t checksum = 0;
  for(size_t i=0; i<length; i++){
    checksum += buf[i];
  }
  return checksum;
}
