#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#include "globals.h"
#include "countdown.h"
#include "protocol.h"
#include "children.h"
#include "client_commands.h"
#include "found_files.h"
#include "servers_with_capacity.h"
#include "client_utils.h"
#include "client_download_utils.h"
#include "client_upload_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>

void terminate () {
  found_files_clear();
  potential_file_receivers_clear();
  children_kill();
  children_clear();
  if (tcp_sock != -1)
    close(tcp_sock);
  if (openfile != -1)
    close(openfile);
  exit(EXIT_SUCCESS);
}

void sigint_handle (int sig) {
  (void) sig; //unused
  terminate();
}

void search (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout, char * const arg) {

  size_t data_len;
  uint64_t seq = next_seq();

  if (arg == NULL)
    data_len = 0;
  else {
    data_len = min(strlen(arg), MAX_SIMPL_DATA_LENGTH);
    strncpy(msg_buf.simpl_data, arg, MAX_SIMPL_DATA_LENGTH);
  }

  if (send_msg(udp_sock, mcast, SIMPL, LIST, seq, 0, data_len) != FAILURE) {

    struct timespec finish;
    struct sockaddr_in server_address;
    socklen_t addrlen = sizeof(server_address);
    char server_addr[INET_ADDRSTRLEN];
    ssize_t len;
    char *strtok_buf;

    found_files_clear();

    for (struct timeval ttl = countdown_start(timeout, &finish);
      ttl.tv_sec || ttl.tv_usec; ttl = countdown_remaining(finish)) {

      setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &ttl, sizeof(ttl));
      len = recv_msg(udp_sock, &server_address, &addrlen);

			if (validate_msg(&msg_buf, seq, len, MY_LIST)) {

        int add_server_result = found_files_add_server(server_address.sin_addr);

        for (char *filename = strtok_r(msg_buf.simpl_data, "\n", &strtok_buf);
          filename != NULL; filename = strtok_r(NULL, "\n", &strtok_buf)) {

          if (add_server_result == FAILURE
            || found_files_add_file(filename) == FAILURE)
            printf("Out of memory - the following filename won't be remembered\n");

          inet_ntop(AF_INET, &server_address.sin_addr, server_addr,
            sizeof(server_addr));
          printf("%s (%s)\n", filename, server_addr);
        }

      } else if (len != -1) {
        skip_package(server_address);
      }
    }
  }
}

void upload (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout, const char * const filepath) {

  struct stat filestat;
  char * filename = path_to_filename(filepath);

  if (stat(filepath, &filestat) == -1 || !S_ISREG(filestat.st_mode))
    printf("File %s does not exist\n", filename);

  else {

    pid_t pid = fork();

    if (pid < 0)
      upload_error(filename, mcast, "Cannot fork a child process");

    else if (pid > 0)
     children_push(pid);

    else {

      int _udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

      close(udp_sock);
      children_clear();

      if (_udp_sock == -1) {
        upload_error(filename, mcast, "Cannot create a UDP socket");
        terminate();
      }

      if (find_potential_file_receivers(_udp_sock, filestat.st_size, mcast,
        timeout) == FAILURE)

        upload_error(filename, mcast,
          "Cannot prepare a list of servers capable of receiving the file");

      else {

        struct sockaddr_in server_address;

        sort_potential_file_receivers_descending();

        for (size_t i = 0; i < potential_file_receivers.size; ++i) {

          server_address = mcast;
          server_address.sin_addr = potential_file_receivers.servers[i].sin_addr;

          if (request_upload(_udp_sock, &server_address, filename, timeout,
            filestat.st_size) != FAILURE) {

            close(_udp_sock);
            send_file(server_address, filepath);
            terminate();
          }
        }

        printf("File %s too big\n", filename);
      }

      close(_udp_sock);
      terminate();
    }
  }
}

void fetch (const int udp_sock, const struct sockaddr_in mcast, const time_t timeout,
  const char * const filename, const char * const out_fldr) {

  struct sockaddr_in server_address = mcast;

  if (find_any_owner(filename, &server_address.sin_addr) == FAILURE)
    printf("File %s not found\n", filename);

  else {

    pid_t pid = fork();

    if (pid < 0)
      download_error(filename, server_address, "Cannot fork a child process");

    else if (pid > 0)
      children_push(pid);

    else {

      close(udp_sock);
      children_clear();

      if (request_download(&server_address, filename, timeout) != FAILURE)
        receive_file(server_address, filename, out_fldr);

      terminate();
    }
  }
}

void discover (const int udp_sock, const struct sockaddr_in mcast,
  const time_t timeout) {

  uint64_t seq = next_seq();

  if (send_msg(udp_sock, mcast, SIMPL, HELLO, seq, 0, 0) != FAILURE) {

    struct timespec finish;
    struct sockaddr_in server_address;
    struct in_addr dump;
    socklen_t addrlen = sizeof(server_address);
    char server_addr[INET_ADDRSTRLEN];
    ssize_t len;

    for (struct timeval ttl = countdown_start(timeout, &finish);
      ttl.tv_sec || ttl.tv_usec; ttl = countdown_remaining(finish)) {

      setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &ttl, sizeof(ttl));
      len = recv_msg(udp_sock, &server_address, &addrlen);

			if (validate_msg(&msg_buf, seq, len, GOOD_DAY)
        && inet_pton(AF_INET, msg_buf.cmplx_data, &dump) != 0) {

        inet_ntop(AF_INET, &server_address.sin_addr, server_addr,
          sizeof(server_addr));

        printf("Found %s (%s) with free space %"PRIu64"\n",
          server_addr, msg_buf.cmplx_data, msg_buf.param);

      } else if (len != -1) {
        skip_package(server_address);
      }
    }
  }
}

void delete (const int udp_sock, const struct sockaddr_in mcast,
  const char * const arg) {

  strncpy(msg_buf.simpl_data, arg, MAX_SIMPL_DATA_LENGTH);

  send_msg(udp_sock, mcast, SIMPL, DELETE, next_seq(), 0,
    strlen(msg_buf.simpl_data));
}
