#ifndef CLIENT_DOWNLOAD_H
#define CLIENT_DOWNLOAD_H

int find_any_owner (const char * const filename, struct in_addr *const owner_buf);

void download_success (const char * const filename,
  const struct sockaddr_in server_address);

void download_error (const char * const filename,
  const struct sockaddr_in server_address, const char * const error_message);

void receive_file (const struct sockaddr_in server_address,
  const char * const filename, const char * const out_fldr);

int request_download (struct sockaddr_in * const server_address,
  const char * const filename, const time_t timeout);

#endif
