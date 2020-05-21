#ifndef CLIENT_UPLOAD_H
#define CLIENT_UPLOAD_H

void upload_error (const char * const filename,
  const struct sockaddr_in server_address, const char * const error_message);

void upload_success (const char * const filename,
  const struct sockaddr_in server_address);

int request_upload (const int udp_sock, struct sockaddr_in * const server_address,
  const char * const filename, const time_t timeout, const off_t filesize);

void send_file (const struct sockaddr_in server_address,
  const char * const filepath);
  
#endif
