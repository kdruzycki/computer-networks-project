#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <netinet/in.h>

#define SEND_TIMEOUT_SEC 0
#define SEND_TIMEOUT_USEC 500000


extern int tcp_sock, openfile;
extern uint64_t last_seq;
extern msg_t msg_buf;

ssize_t write_with_retry (const int fd, const void* buf, size_t size);

uint64_t next_seq ();

int send_msg (const int udp_sock, const struct sockaddr_in remote_address,
  const int msg_type, const char * const msg,
  const uint64_t seq, const uint64_t param, const size_t data_len);

ssize_t recv_msg (const int udp_sock, struct sockaddr_in * const src_addr,
  socklen_t * const addrlen);

void skip_package (const struct sockaddr_in server_address);

char * path_to_filename(const char * const path);

size_t calc_filepath_size(const char * const fldr, const char * const filename);

int filename_to_filepath(const char * const fldr, const char * const filename,
  char * const buffer, const size_t buffer_size);

#endif
