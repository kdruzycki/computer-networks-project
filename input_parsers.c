#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#include <stddef.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "input_parsers.h"
#include "globals.h"

int parse_cmd (char * const cmd, char **arg) {

  char *name, *strtok_buf;

  if (cmd == NULL)
    return UNKNOWN;

  name = strtok_r(cmd, " \n", &strtok_buf);
  *arg = strtok_r(NULL, "\n", &strtok_buf);

  if (name == NULL)
    return UNKNOWN;

  if (!strcasecmp("discover", name) && (*arg == NULL || strlen(*arg) == 0))
    return DISCOVER;
  if (!strcasecmp("search", name))
    return SEARCH;
  if (!strcasecmp("fetch", name) && *arg != NULL)
    return FETCH;
  if (!strcasecmp("upload", name) && *arg != NULL)
    return UPLOAD;
  if (!strcasecmp("remove", name) && *arg != NULL)
    return REMOVE;
  if (!strcasecmp("exit", name) && (*arg == NULL || strlen(*arg) == 0))
    return CLOSE;

  return UNKNOWN;
}

int parse_arg (int argc, char *argv[],
  struct in_addr *mcast_addr, in_port_t *cmd_port,
  char **out_fldr, time_t *timeout) {

  int was_addr = 0, was_port = 0, was_fldr = 0;

  *timeout = DEFAULT_TIMEOUT;

  if (argc%2 == 0)
    return FAILURE;

  for (int i = 1; i < argc-1; i+=2) {

    if (argv[i][0] != '-' || argv[i][1] == '\0' || argv[i][2] != '\0')
      return FAILURE;

    switch (argv[i][1]) {

      case 'p':
        was_port = 1;
        *cmd_port = (in_port_t) atoi(argv[i+1]);
        break;

      case 'g':
        was_addr = 1;
        if (inet_pton(AF_INET, argv[i+1], mcast_addr) == 0)
          return FAILURE;
        break;

      case 'o':
        was_fldr = 1;
        *out_fldr = argv[i+1];
        break;

      case 't':
        *timeout = atoi(argv[i+1]);
        if (*timeout <= 0 || *timeout > 300)
          return FAILURE;
        break;

      default:
        return FAILURE;
    }
  }

  if (!was_addr || !was_port || !was_fldr)
    return FAILURE;

  return SUCCESS;
}
