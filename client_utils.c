#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "globals.h"
#include "protocol.h"
#include "client_utils.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

int tcp_sock = -1, openfile = -1;
uint64_t last_seq = UINT64_MAX;
msg_t msg_buf;

ssize_t write_with_retry (const int fd, const void* buf, size_t size) {

  ssize_t ret = 0, part;

  while (size > 0) {

    do
      part = write(fd, buf, size);
    while ((part < 0) && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK));

    if (part < 0)
      return ret;

    ret += part;
    size -= part;
    buf += part;
  }
  return ret;
}

uint64_t next_seq () {

  if (last_seq == UINT64_MAX) {
    srandom(time(NULL));
    last_seq = random()%UINT64_MAX;

  } else
    ++last_seq;

  return last_seq;
}

int send_msg (const int udp_sock, const struct sockaddr_in remote_address,
  const int msg_type, const char * const msg,
  const uint64_t seq, const uint64_t param, const size_t data_len) {

  ssize_t snd_len;
  size_t msg_len;
  struct timeval tv = {.tv_sec = SEND_TIMEOUT_SEC, .tv_usec = SEND_TIMEOUT_USEC};

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstringop-truncation"
  strncpy(msg_buf.header.cmd, msg, MSG_LENGTH);
  #pragma GCC diagnostic pop
  msg_buf.header.cmd_seq = htobe64(seq);
  if (msg_type == CMPLX)
      msg_buf.param = htobe64(param);
  msg_len = calc_msg_len(&msg_buf, data_len);

  setsockopt(udp_sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  snd_len = sendto(udp_sock, &msg_buf, msg_len, 0,
    (struct sockaddr *) &remote_address, sizeof(remote_address));

  if (snd_len != (ssize_t) msg_len)
    return FAILURE;

  return SUCCESS;
}

ssize_t recv_msg (const int udp_sock, struct sockaddr_in * const src_addr,
  socklen_t * const addrlen) {

  ssize_t len;

  msg_buf.simpl_data[0] = '\0';
  msg_buf.cmplx_data[0] = '\0';

  len = recvfrom(udp_sock, &msg_buf, sizeof(msg_buf)-1, 0,
    (struct sockaddr *) src_addr, addrlen);

  if (len > 0) {

    ((char *) &msg_buf)[len] = '\0';

    if (get_msg_type(&msg_buf) == CMPLX)
      msg_buf.param = be64toh(msg_buf.param);

    msg_buf.header.cmd_seq = be64toh(msg_buf.header.cmd_seq);
  }

  return len;
}

void skip_package (const struct sockaddr_in server_address) {

  char server_addrstr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &server_address.sin_addr,
    server_addrstr, sizeof(server_addrstr));

  fprintf(stderr, "[PCKG ERROR]  Skipping invalid package from (%s:%hu).\n",
    server_addrstr, ntohs(((struct sockaddr_in *)&server_address)->sin_port));
}

char * path_to_filename(const char * const path) {

  int i;

  for (i = strlen(path); i > 0; --i)
    if (path[i-1] == '/')
      break;

  return (char *) path+i;
}

size_t calc_filepath_size(const char * const fldr, const char * const filename) {

  if (fldr[strlen(fldr)-1] == '/')
    return strlen(fldr)+strlen(filename)+1;
  else
    return strlen(fldr)+strlen(filename)+2;
}

int filename_to_filepath(const char * const fldr, const char * const filename,
  char * const buffer, const size_t buffer_size) {

  if (buffer_size >= calc_filepath_size(fldr, filename)) {

    strncpy(buffer, fldr, buffer_size);
    if (buffer[strlen(buffer)-1] != '/')
      buffer[strlen(buffer)] = '/';
    strcat(buffer, filename);

    return SUCCESS;

  } else
    return FAILURE;
}
