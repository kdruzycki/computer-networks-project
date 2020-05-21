#ifndef SERVERS_WITH_CAPACITY_H
#define SERVERS_WITH_CAPACITY_H

#include <inttypes.h>
#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>

#define INITIAL_VECTOR_MAX_SIZE 16

typedef struct {
  struct in_addr sin_addr;
  uint64_t capacity;
} server_entry;

typedef struct {
  server_entry *servers;
  size_t max_size;
  size_t size;
} server_entries;

extern server_entries potential_file_receivers;
extern server_entry *potential_file_receivers_tmp;

int server_capacity_compare_desc (const void * a, const void * b);

void sort_potential_file_receivers_descending ();

void potential_file_receivers_clear ();

int potential_file_receivers_resize (const size_t new_size);

int potential_file_receivers_push (struct in_addr sin_addr, uint64_t capacity);

int find_potential_file_receivers (const int udp_sock,
  const off_t min_server_capacity, const struct sockaddr_in mcast,
  const time_t timeout);

#endif
