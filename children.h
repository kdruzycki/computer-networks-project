#ifndef CHILDREN_H
#define CHILDREN_H

#include <sys/types.h>

typedef struct pid_list_struct {
  int pid;
  struct pid_list_struct *next;
} pid_list;

extern pid_list *children, *children_tmp;

int children_push (const pid_t pid);

void children_clear ();

void children_kill ();

#endif
