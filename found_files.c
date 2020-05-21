#include "globals.h"
#include "found_files.h"

#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

servers_files *found_files = NULL;

//Must be global for the sigint_handle - in case new list element gets created
//but program is interrupted before the element gets attached to the list
servers_files *found_files_tmp = NULL;
file_list *files_tmp = NULL;
char *filename_tmp_ptr = NULL;

void file_list_clear (file_list *list) {

  for (file_list *next = NULL; list != NULL; list = next) {
    next = list->next;

    if (list->filename == filename_tmp_ptr)
      filename_tmp_ptr = NULL; //so that no double free occurs in found files clear
    if (list == files_tmp)
      files_tmp = NULL;

    free(list->filename);
    free(list);
  }
}

void found_files_clear () {

  if (found_files == found_files_tmp)
    found_files_tmp = NULL;

  for (servers_files *next = NULL; found_files != NULL; found_files = next) {

    next = found_files->next;
    file_list_clear(found_files->files);
    free(found_files);
  }

  free(found_files_tmp);

  free(filename_tmp_ptr);
  free(files_tmp);
}

int found_files_add_server (const struct in_addr server_address) {

  found_files_tmp = malloc(sizeof(servers_files));
  if (found_files_tmp == NULL)
    return FAILURE;

  else {
    found_files_tmp->server_address = server_address;
    found_files_tmp->next = found_files;
    found_files_tmp->files = NULL;
    found_files = found_files_tmp;

    return SUCCESS;
  }
}

int found_files_add_file (const char * const buffer_with_filename) {

  if (found_files == NULL)
    return FAILURE;

  else {

    files_tmp = malloc(sizeof(file_list));
    if (files_tmp == NULL)
      return FAILURE;

    filename_tmp_ptr = malloc((strlen(buffer_with_filename)+1)*sizeof(char));
    if (filename_tmp_ptr == NULL) {
      free(files_tmp);
      return FAILURE;
    }

    strcpy(filename_tmp_ptr, buffer_with_filename);

    files_tmp->next = found_files->files;
    files_tmp->filename = filename_tmp_ptr;
    found_files->files = files_tmp;

    return SUCCESS;
  }
}
