#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif

#include "globals.h"
#include "children.h"

#include <signal.h>
#include <stdlib.h>

pid_list *children = NULL;

//Must be global for the sigint_handle - in case new list element gets created
//but program is interrupted before the element gets attached to the list
pid_list *children_tmp = NULL;

int children_push (const pid_t pid) {

  children_tmp = malloc(sizeof(pid_list));
  if (children_tmp == NULL)
    return FAILURE;

  else {
    children_tmp->pid = pid;
    children_tmp->next = children;
    children = children_tmp;

    return SUCCESS;
  }
}

void children_clear () {

  if (children_tmp == children)
    children_tmp = NULL;

  for (pid_list *next = NULL; children != NULL; children = next) {

    next = children->next;
    free(children);
  }

  free(children_tmp);
}

void children_kill () {

  for (pid_list *next = NULL, *list = children; list != NULL; list = next) {

    next = list->next;
    kill(list->pid, SIGINT);
  }
}
