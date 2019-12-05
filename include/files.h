#ifndef FILES_H
#define FILES_H

#include "buffer.h"
#include "definitions.h"

#include <dirent.h>
#include <sys/stat.h>

int open_file(buffer* b, int flags);
DIR* open_dir(buffer* b);
int get_file_stat(buffer* b, struct stat* statbuf);
char* get_content_type(buffer* filename);

void insert_filename_or_dirname(buffer* out, buffer path, char** data, unsigned n, int is_file);

void sort_filenames(char** filenames, int n);

void insert_http_content_response_header(buffer* response_buffer, buffer content_type, u64 content_length);

#endif
