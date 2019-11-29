#ifndef BUFFER_H
#define BUFFER_H

#include "definitions.h"

typedef struct {
    s8* data;
    u64 length;
    u64 capacity;
} buffer;

buffer make_buffer(int capacity);
void free_buffer(buffer* b);

void buffer_realloc(buffer* b, unsigned n);
void buffer_ensure_null(buffer* b);

void buffer_insert_bytes(buffer* b, s8* bytes, unsigned n);
void buffer_insert_char(buffer* b, char c);
void buffer_insert_string(buffer* b, const char* str);
void buffer_insert_uint(buffer* b, unsigned int num);
void buffer_insert_current_date(buffer* b);

#endif
