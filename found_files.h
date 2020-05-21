#ifndef FOUND_FILES_H
#define FOUND_FILES_H

#include <arpa/inet.h>

typedef struct filename_list {
  char *filename;
  struct filename_list *next;
} file_list;

typedef struct server_files_list {
  struct in_addr server_address;
  file_list *files;
  struct server_files_list *next;
} servers_files;

extern servers_files *found_files, *found_files_tmp;
extern file_list *files_tmp;
extern char *filename_tmp_ptr;

void file_list_clear (file_list *list);

void found_files_clear ();

int found_files_add_server (const struct in_addr server_address);

int found_files_add_file (const char * const buffer_with_filename);

#endif
