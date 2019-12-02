#ifndef FILES_H
#define FILES_H

#include "buffer.h"

#include <dirent.h>
#include <sys/stat.h>

int open_file(buffer* b, int flags);
DIR* open_dir(buffer* b);
int get_file_stat(buffer* b, struct stat* statbuf);
char* get_content_type(buffer* filename);

void sort_filenames(char** filenames, int n);

#endif
