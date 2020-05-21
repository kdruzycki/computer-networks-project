#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#include <time.h>
#include <sys/time.h>

struct timeval countdown_start (const time_t ttl, struct timespec * const end_buf) {

  clock_gettime(CLOCK_MONOTONIC, end_buf);
  end_buf->tv_sec += ttl;

  return (struct timeval) {
    .tv_sec = ttl,
    .tv_usec = 0
  };
}

struct timeval countdown_remaining (const struct timespec end) {

  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &now);

  if (end.tv_sec < now.tv_sec ||
    (end.tv_sec == now.tv_sec && end.tv_nsec <= now.tv_nsec))
    return (struct timeval) {0, 0};

  else
    return (struct timeval) {
      .tv_sec = end.tv_sec-now.tv_sec,
      .tv_usec = (end.tv_nsec-now.tv_nsec)/1000
    };
}
