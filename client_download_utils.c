#include "globals.h"
#include "protocol.h"
#include "countdown.h"
#include "found_files.h"
#include "client_utils.h"
#include "client_download_utils.h"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int find_any_owner (const char * const filename, struct in_addr * const owner_buf) {

  if (owner_buf != NULL)
    for (servers_files *i = found_files; i != NULL; i = i->next)
      for (file_list *j = i->files; j != NULL; j = j->next)
        if (!strcmp(j->filename, filename)) {
          *owner_buf = i->server_address;
          return SUCCESS;
        }

  return FAILURE;
}

void download_success (const char * const filename,
  const struct sockaddr_in server_address) {

  char server_addr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &server_address.sin_addr,
    server_addr, sizeof(server_addr));

  printf("File %s downloaded (%s:%hu)\n", filename, server_addr,
    ntohs(server_address.sin_port));
}

void download_error (const char * const filename,
  const struct sockaddr_in server_address, const char * const error_message) {

  char server_addrstr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &server_address.sin_addr,
    server_addrstr, sizeof(server_addrstr));

  printf("File %s downloading failed (%s:%hu) %s\n", filename, server_addrstr,
    ntohs(server_address.sin_port), error_message);
}

void receive_file (const struct sockaddr_in server_address,
  const char * const filename, const char * const out_fldr) {

  tcp_sock = socket(AF_INET, SOCK_STREAM, 0);

  if (connect(tcp_sock, (struct sockaddr*) &server_address, sizeof(server_address)) == -1)
    download_error(filename, server_address, "Cannot establish connection");

  else {

    struct stat dirstat;

    if (stat(out_fldr, &dirstat) == -1
      && mkdir(out_fldr, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
        download_error(filename, server_address,
          "Unable to create download directory");

    else if (stat(out_fldr, &dirstat) != -1
      && chmod(out_fldr, dirstat.st_mode | S_IRWXU) == -1)
      download_error(filename, server_address,
        "Cannot set proper permissions for the output folder");

    else if (!S_ISDIR(dirstat.st_mode))
      download_error(filename, server_address,
        "Download directory is not in fact a directory");

    else {

      ssize_t length;
      char transfer_buf[FILE_TRANSFER_BUFFER_SIZE];
      char filepath[calc_filepath_size(out_fldr, filename)];

      filename_to_filepath(out_fldr, filename, filepath, sizeof(filepath));
      openfile = creat(filepath, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

      if (openfile == -1)
        download_error(filepath, server_address,
          "Cannot create or overwrite the local file");

      else {

        int was_error = 0;

        do {

          length = read(tcp_sock, transfer_buf, sizeof(transfer_buf));

          if (length < 0) {

            download_error(filename, server_address,
              "Unable to read from TCP socket");

            was_error = 1;
          }

          else if (length > 0
            && write_with_retry(openfile, transfer_buf, length) != length) {

            download_error(filename, server_address,
              "Unable to write to the local output file");

            was_error = 1;
          }

        } while (length > 0 && !was_error);

        close(openfile);
        openfile = -1;

        if (!was_error)
          download_success(filename, server_address);
      }
    }
  }

  close(tcp_sock);
  tcp_sock = -1;
}

int request_download (struct sockaddr_in * const server_address,
  const char * const filename, const time_t timeout) {

  uint64_t seq = next_seq();
  size_t data_len = min(strlen(filename), MAX_SIMPL_DATA_LENGTH);
  int _udp_sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (_udp_sock == -1) {
    download_error(filename, *server_address, "Cannot create a UDP socket");
    return FAILURE;
  }

  strncpy(msg_buf.simpl_data, filename, MAX_SIMPL_DATA_LENGTH);

  if (send_msg(_udp_sock, *server_address, SIMPL, GET, seq, 0, data_len) == FAILURE) {

    download_error(filename, *server_address,
      " Unable to send a download request via socket");

  } else {

    struct timespec finish;
    struct sockaddr_in message_sender;
    socklen_t addrlen = sizeof(message_sender);
    ssize_t len;

    for (struct timeval ttl = countdown_start(timeout, &finish);
      ttl.tv_sec || ttl.tv_usec; ttl = countdown_remaining(finish)) {

      setsockopt(_udp_sock, SOL_SOCKET, SO_RCVTIMEO, &ttl, sizeof(ttl));
      len = recv_msg(_udp_sock, &message_sender, &addrlen);

      if (validate_msg(&msg_buf, seq, len, CONNECT_ME)
        && message_sender.sin_addr.s_addr == server_address->sin_addr.s_addr
        && message_sender.sin_port == server_address->sin_port
        && msg_buf.param <= USHRT_MAX
        && !strncmp(msg_buf.cmplx_data, filename, MAX_CMPLX_DATA_LENGTH)) {

        server_address->sin_port = htons((in_port_t) msg_buf.param);

        close(_udp_sock);
        return SUCCESS;

      } else if (len != -1)
        skip_package(message_sender);

    }

    download_error(filename, *server_address, " Server not responding");
  }

  close(_udp_sock);
  return FAILURE;
}
