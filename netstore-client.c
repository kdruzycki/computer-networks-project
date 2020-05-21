#include "globals.h"
#include "protocol.h"
#include "input_parsers.h"
#include "client_commands.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>

int main (int argc, char *argv[]) {

  char *out_fldr;
  time_t timeout;
  in_port_t cmd_port;
  struct in_addr mcast_addr;

  char cmd_line[MAX_COMMAND_LENGTH+1], *arg;
  int udp_sock = -1;
  struct sockaddr_in mcast;

  if (parse_arg(argc, argv, &mcast_addr, &cmd_port, &out_fldr, &timeout) < 0) {
    fprintf(stderr, "Incorrect or too few or too many parameters\n");
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, sigint_handle);

  udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  while (udp_sock == -1) {
    printf("Error trying to create a UDP socket, retrying...\n");
    sleep(1);
    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
  }

  mcast.sin_family = AF_INET;
  mcast.sin_addr = mcast_addr;
  mcast.sin_port = htons(cmd_port);

  for (;;) {
    if (fgets(cmd_line, MAX_COMMAND_LENGTH, stdin) != NULL) {
      switch (parse_cmd(cmd_line, &arg)) {
        case DISCOVER:
          discover(udp_sock, mcast, timeout);
          break;
        case SEARCH:
          search(udp_sock, mcast, timeout, arg);
          break;
        case FETCH:
          fetch(udp_sock, mcast, timeout, arg, out_fldr);
          break;
        case UPLOAD:
          upload(udp_sock, mcast, timeout, arg);
          break;
        case REMOVE:
          delete(udp_sock, mcast, arg);
          break;
        case CLOSE:
          terminate();
          break;
      }
    }
  }

  return 0;
}
