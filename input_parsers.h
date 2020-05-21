#ifndef INPUT_PARSERS_H
#define INPUT_PARSERS_H

#define MAX_COMMAND_LENGTH 65500
#define DEFAULT_TIMEOUT 5

#define DISCOVER 1
#define SEARCH 2
#define FETCH 3
#define UPLOAD 4
#define REMOVE 5
#define CLOSE 6
#define UNKNOWN -1

#include <time.h>
#include <netinet/in.h>

int parse_cmd (char * const cmd, char **arg);

int parse_arg (int argc, char *argv[],
  struct in_addr *mcast_addr, in_port_t *cmd_port,
  char **out_fldr, time_t *timeout);

#endif
