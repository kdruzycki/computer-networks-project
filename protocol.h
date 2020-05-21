#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <inttypes.h>
#include <unistd.h>

#define MAX_SIMPL_DATA_LENGTH 65488
#define MAX_CMPLX_DATA_LENGTH (MAX_SIMPL_DATA_LENGTH-sizeof(uint64_t))
#define MSG_LENGTH 10

#define SIMPL 0
#define CMPLX 1

#define HELLO "HELLO"
#define GOOD_DAY "GOOD_DAY"
#define LIST "LIST"
#define MY_LIST "MY_LIST"
#define GET "GET"
#define CAN_ADD "CAN_ADD"
#define DELETE "DEL"
#define ADD "ADD"
#define CONNECT_ME "CONNECT_ME"
#define NO_WAY "NO_WAY"

typedef struct __attribute__((packed)) {
  struct  __attribute__((packed)) {
    char cmd[MSG_LENGTH];
    uint64_t cmd_seq;
  } header;
  union __attribute__((packed)) {
    struct __attribute__((packed)) {
      uint64_t param;
      char cmplx_data[MAX_CMPLX_DATA_LENGTH+1];
    };
    char simpl_data[MAX_SIMPL_DATA_LENGTH+1];
  };
} msg_t;

extern msg_t msg_buf;

int get_msg_type(const msg_t * const msg_buf);

size_t calc_msg_len (const msg_t * const msg_buf, const size_t data_len);

size_t calc_data_len (const msg_t * const msg_buf, const size_t msg_len);

int validate_msg (const msg_t * const msg_buf, const uint64_t expected_seq,
  const size_t msg_len, const char * const expected_cmd);

#endif
