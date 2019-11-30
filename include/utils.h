#ifndef UTILS_H
#define UTILS_H

#include "buffer.h"

int find_http_newline(const char* str, int n);
int find_http_header_end(const char* str, int n);

int ltrim_space(const char* str, int n);
int ltrim_http_newline(const char* str, int n);

int find_from_char_set(const char* str, const char* char_set, int n);
int iequals(const char* a, unsigned len_a, const char* b);
int increment_array_pointer(char** array, u64* len, s64 increment);

void set_http_error_response(buffer* b, const char* headers, const char* body);

http_method get_http_method(buffer b);

#endif
