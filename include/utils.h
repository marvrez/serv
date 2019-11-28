#ifndef UTILS_H
#define UTILS_H

int get_http_newline_length(const char* str, int n);
int get_http_header_length(const char* str, int n);

int ltrim_space(const char* str, int n);
int ltrim_http_newline(const char* str, int n);

int find_from_char_set(const char* str, const char* char_set, int n);
int iequals(const char* a, const char* b);
int increment_array_pointer(char** array, int* len, int increment);

#endif
