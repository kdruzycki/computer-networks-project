#include "protocol.h"

#include <string.h>
#include <inttypes.h>
#include <unistd.h>

msg_t msg_buf;

int get_msg_type(const msg_t * const msg_buf) {

  if (!strncmp(msg_buf->header.cmd, GOOD_DAY, MSG_LENGTH) ||
   !strncmp(msg_buf->header.cmd, ADD, MSG_LENGTH) ||
   !strncmp(msg_buf->header.cmd, CAN_ADD, MSG_LENGTH) ||
   !strncmp(msg_buf->header.cmd, CONNECT_ME, MSG_LENGTH))
    return CMPLX;

  return SIMPL;
}

size_t calc_msg_len (const msg_t * const msg_buf, const size_t data_len) {

  return (get_msg_type(msg_buf) == CMPLX)
    ?sizeof(msg_buf->header)+sizeof(msg_buf->param)+data_len
    :sizeof(msg_buf->header)+data_len;
}

size_t calc_data_len (const msg_t * const msg_buf, const size_t msg_len) {

  size_t pref_len = (get_msg_type(msg_buf) == CMPLX)
    ?sizeof(msg_buf->header)+sizeof(msg_buf->param)
    :sizeof(msg_buf->header);

  return (pref_len > msg_len)
    ?0
    :(msg_len-pref_len);
}

int validate_msg (const msg_t * const msg_buf, const uint64_t expected_seq,
  const size_t msg_len, const char * const expected_cmd) {

  if (expected_cmd == NULL ||
    strncmp(msg_buf->header.cmd, expected_cmd, MSG_LENGTH) != 0)
      return 0;
  else
    return msg_len >= calc_msg_len(msg_buf, 0)
      && msg_buf->header.cmd_seq == expected_seq;
}
