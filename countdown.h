#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <sys/time.h>
#include <time.h>

struct timeval countdown_start (const time_t ttl, struct timespec * const end_buf);

struct timeval countdown_remaining (const struct timespec end);

#endif
