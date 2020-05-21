#ifndef COMMANDS_H
#define COMMANDS_H

#include <netinet/in.h>

extern msg_t msg_buf;
extern int tcp_sock, openfile;

void terminate ();

void sigint_handle (int sig);

void search (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout, char * const arg);

void upload (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout, const char * const filepath);

void fetch (const int udp_sock, const struct sockaddr_in mcast, const time_t timeout,
  const char * const filename, const char * const out_fldr);

void discover (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout);

void delete (const int udp_sock, const struct sockaddr_in mcast,
  const char * const arg);

#endif
