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
void test_s3p_build_with_large_payload();

int main(int argc, char** argv){
  printf("Super Simple Streaming Protocol Test Program\n");

  test_s3p_build_no_escape();
  test_s3p_build_with_escape();
  test_s3p_build_with_small_buf();
  test_s3p_build_with_large_payload();
  test_s3p_read_no_escape();
  test_s3p_read_with_escape();
  test_s3p_read_without_start_byte();
  test_s3p_read_with_mixed_packet();
  test_s3p_read_bad_checksum();
  test_s3p_read_truncated_packet();
  test_s3p_read_empty_packet();

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
  int psize;
  err = s3p_build(data, sizeof data, out, 20, &psize);
  CHECK(S3P_SUCCESS == err);
  CHECK(7 == psize);
  uint8_t template[] = { 0x56, 0x04, 0x00, 0x01, 0x02, 0x03, 0x06 };
  int i;
  for(i=0; i<psize; i++){
    //    printf("%02X ", out[i]);
    CHECK(out[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_with_escape(){
  printf("Testing s3p_build(), with escaping. ");
  uint8_t data[] = { 0x25, 0x01, 0x56, 0x03 };
  uint8_t out[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_build(data, sizeof data, out, 20, &psize);
  CHECK(S3P_SUCCESS == err);
  CHECK(9 == psize);
  uint8_t template[] = { 0x56, 0x04, 0x25, 0x05, 0x01, 0x25, 0x76, 0x03, 0x7F };
  int i;
  for(i=0; i<psize; i++){
    //    printf("%02X ", out[i]);
    CHECK(out[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_with_small_buf(){
  printf("Testing s3p_build(), with small buffer. ");
  uint8_t data[] = { 0x25, 0x01, 0x56, 0x03 };
  uint8_t out[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_build(data, sizeof data, out, 1, &psize);
  CHECK(S3P_BUF_TOO_SMALL == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_build_with_large_payload(){
  printf("Testing s3p_build(), with oversize payload. ");
  uint8_t data[300] = { 0x00 };
  uint8_t out[350] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_build(data, sizeof data, out, 350, &psize);
  CHECK(S3P_PAYLOAD_TOO_LARGE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_no_escape(){
  printf("Testing s3p_read(), no escaping. ");
  uint8_t in[] = { 0x56, 0x04, 0x00, 0x01, 0x02, 0x03, 0x06 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  
  CHECK(S3P_SUCCESS == err);
  CHECK(4 == psize);
  
  int i;
  for(i=0; i<psize; i++){
    CHECK(data[i] == i);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_with_escape(){
  printf("Testing s3p_read(), with escaping. ");
  uint8_t in[] = { 0x56, 0x04, 0x25, 0x05, 0x01, 0x25, 0x76, 0x03, 0x7F };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);

  CHECK(S3P_SUCCESS == err);
  CHECK(4 == psize);

  uint8_t template[] = { 0x25, 0x01, 0x56, 0x03 };
  int i;
  for(i=0; i<psize; i++){
    CHECK(data[i] == template[i]);
  }
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_with_mixed_packet(){
  printf("Testing s3p_read(), with mixed packet. ");
  uint8_t in[] = { 0x56, 0x04, 0x25, 0x56, 0x01, 0x25, 0x76, 0x03, 0x7F };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);  
}

void test_s3p_read_without_start_byte(){
  printf("Testing s3p_read(), with no start byte. ");
  uint8_t in[] = { 0x00, 0x04, 0x25, 0x56, 0x01, 0x25, 0x76, 0x03, 0x7F };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_bad_checksum(){
  printf("Testing s3p_read(), with bad checksum. ");
  uint8_t in[] = { 0x56, 0x03, 0x01, 0x76, 0x03, 0x70 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_CHECKSUM_ERR == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_truncated_packet(){
  printf("Testing s3p_read(), with truncated packet. ");
  uint8_t in[] = { 0x56, 0x09, 0x01, 0x76, 0x03, 0x70 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, sizeof in, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}

void test_s3p_read_empty_packet(){
  printf("Testing s3p_read(), with empty packet. ");
  uint8_t in[5] = { 0x00 };
  uint8_t data[20] = { 0x00 };

  S3P_ERR err;
  int psize;
  err = s3p_read(in, 0, data, 20, &psize);
  CHECK(S3P_PARSE_FAILURE == err);
  COLOR("Pass\n", GREEN);
}
