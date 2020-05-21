#include "globals.h"
#include "countdown.h"
#include "protocol.h"
#include "servers_with_capacity.h"
#include "client_utils.h"

#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

server_entries potential_file_receivers = {.servers = NULL, .max_size = 0, .size = 0};
server_entry *potential_file_receivers_tmp = NULL;

int server_capacity_compare_desc (const void * a, const void * b) {

    server_entry _a = *(server_entry*)a;
    server_entry _b = *(server_entry*)b;

    if(_a.capacity < _b.capacity) return 1;
    else if(_a.capacity == _b.capacity) return 0;
    else return -1;
}

void sort_potential_file_receivers_descending () {

  qsort(potential_file_receivers.servers,
    potential_file_receivers.size,
    sizeof(server_entry),
    server_capacity_compare_desc);
}


void potential_file_receivers_clear () {

  if (potential_file_receivers_tmp != potential_file_receivers.servers)
    free(potential_file_receivers_tmp);
  potential_file_receivers_tmp = NULL;

  free(potential_file_receivers.servers);
  potential_file_receivers.servers = NULL;
}

int potential_file_receivers_resize (const size_t new_size) {

  potential_file_receivers_tmp = realloc(potential_file_receivers.servers,
    new_size*sizeof(server_entry));

  if (potential_file_receivers_tmp == NULL)
    return FAILURE;

  potential_file_receivers.servers = potential_file_receivers_tmp;
  potential_file_receivers.max_size = new_size;

  return SUCCESS;
}

int potential_file_receivers_push (struct in_addr sin_addr, uint64_t capacity) {

  if (potential_file_receivers.max_size == 0
    && potential_file_receivers_resize(INITIAL_VECTOR_MAX_SIZE) == FAILURE)
    return FAILURE;

  if (potential_file_receivers.max_size <= potential_file_receivers.size
    && (potential_file_receivers_resize(potential_file_receivers.max_size*2) == FAILURE
    || potential_file_receivers_resize(potential_file_receivers.max_size+1) == FAILURE))
    return FAILURE;

  potential_file_receivers.servers[potential_file_receivers.size].sin_addr = sin_addr;
  potential_file_receivers.servers[potential_file_receivers.size].capacity = capacity;
  potential_file_receivers.size++;

  return SUCCESS;
}

int find_potential_file_receivers (const int udp_sock,
  const off_t min_server_capacity, const struct sockaddr_in mcast,
  const time_t timeout) {

  uint64_t seq = next_seq();

  if (send_msg(udp_sock, mcast, SIMPL, HELLO, seq, 0, 0) != FAILURE) {

    struct timespec finish;
    struct sockaddr_in server_address;
    struct in_addr dump;
    socklen_t addrlen = sizeof(server_address);
    ssize_t len;

    for (struct timeval ttl = countdown_start(timeout, &finish);
      ttl.tv_sec || ttl.tv_usec; ttl = countdown_remaining(finish)) {

      setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &ttl, sizeof(ttl));
      len = recv_msg(udp_sock, &server_address, &addrlen);

			if (validate_msg(&msg_buf, seq, len, GOOD_DAY)
        && inet_pton(AF_INET, msg_buf.cmplx_data, &dump) != 0) {

        if (msg_buf.param >= (uint64_t) min_server_capacity)
          if (potential_file_receivers_push(server_address.sin_addr, msg_buf.param)
            == FAILURE)
            return FAILURE;

      } else if (len != -1) {
        skip_package(server_address);
      }
    }

    return SUCCESS;
  }

  return FAILURE;
}
