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

void upload_error (const char * const filename,
  const struct sockaddr_in server_address, const char * const error_message) {

  char server_addrstr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &server_address.sin_addr,
    server_addrstr, sizeof(server_addrstr));

  printf("File %s uploading failed (%s:%hu) %s\n", filename, server_addrstr,
    ntohs(server_address.sin_port), error_message);
}

void upload_success (const char * const filename,
  const struct sockaddr_in server_address) {

  char server_addr[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &server_address.sin_addr,
    server_addr, sizeof(server_addr));

  printf("File %s uploaded (%s:%hu)\n", filename, server_addr,
    ntohs(server_address.sin_port));
}

int request_upload (const int udp_sock, struct sockaddr_in * const server_address,
  const char * const filename, const time_t timeout, const off_t filesize) {

  uint64_t seq = next_seq();
  size_t data_len = min(strlen(filename), MAX_CMPLX_DATA_LENGTH);

  strncpy(msg_buf.cmplx_data, filename, MAX_CMPLX_DATA_LENGTH);

  if (send_msg(udp_sock, *server_address, CMPLX, ADD, seq, filesize, data_len) == FAILURE) {
    upload_error(filename, *server_address,
      "Unable to send an upload request via socket");
    return FAILURE;

  } else {

    struct timespec finish;
    struct sockaddr_in message_sender;
    socklen_t addrlen = sizeof(message_sender);
    ssize_t len;

    for (struct timeval ttl = countdown_start(timeout, &finish);
      ttl.tv_sec || ttl.tv_usec; ttl = countdown_remaining(finish)) {

      setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &ttl, sizeof(ttl));
      len = recv_msg(udp_sock, &message_sender, &addrlen);

      if (validate_msg(&msg_buf, seq, len, CAN_ADD)
        && message_sender.sin_addr.s_addr == server_address->sin_addr.s_addr
        && message_sender.sin_port == server_address->sin_port
        && msg_buf.param <= USHRT_MAX && calc_data_len(&msg_buf, len) == 0) {

        server_address->sin_port = htons((in_port_t) msg_buf.param);
        return SUCCESS;

      } else if (validate_msg(&msg_buf, seq, len, NO_WAY)
        && message_sender.sin_addr.s_addr == server_address->sin_addr.s_addr
        && message_sender.sin_port == server_address->sin_port
        && !strncmp(msg_buf.simpl_data, filename, MAX_SIMPL_DATA_LENGTH)) {

        return FAILURE;

      } else if (len != -1)
        skip_package(message_sender);
    }

    return FAILURE;
  }

  return FAILURE;
}

void send_file (const struct sockaddr_in server_address,
  const char * const filepath) {

  tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
  char *filename = path_to_filename(filepath);

  if (connect(tcp_sock, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
    upload_error(filename, server_address, "Cannot establish connection");

  else {

    ssize_t length;

    openfile = open(filepath, O_RDONLY);
    char transfer_buf[FILE_TRANSFER_BUFFER_SIZE];

    if (openfile == -1)
      upload_error(filename, server_address,
        "Cannot open the local file");

    else {

      int was_error = 0;

      do {

        length = read(openfile, transfer_buf, sizeof(transfer_buf));

        if (length < 0) {

          upload_error(filename, server_address,
            "Unable to read from file");

          was_error = 1;

        } else if (length > 0
          && write_with_retry(tcp_sock, transfer_buf, length) != length) {

          upload_error(filename, server_address,
            "Unable to write to TCP socket");

          was_error = 1;
        }

      } while (length > 0 && !was_error);

      close(openfile);
      openfile = -1;

      if (!was_error)
        upload_success(filename, server_address);
    }
  }

  close(tcp_sock);
  tcp_sock = -1;
}
