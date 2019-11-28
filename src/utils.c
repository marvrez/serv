#include "utils.h"

#include <string.h>
#include <ctype.h>

int get_http_newline_length(const char* str, int n)
{
    if(n < 1) return 0;
    if(str[0] == '\n') return 1;
    if(n > 1 && str[0] == '\r' && str[1] == '\n') return 2;
    return 0;
}

int get_http_header_length(const char* str, int n)
{
    if(n < 2) return 0;
    int i = get_http_newline_length(str, n);
    if(i == 0) return 0;
    int j = get_http_newline_length(str + i, n - i);
    if(j == 0) return 0;
    return i + j;
}

int ltrim_space(const char* str, int n)
{
    int i = 0;
    for(; i < n && (str[i] == ' ' || str[i] == '\t'); ++i);
    return i;
}

int ltrim_http_newline(const char* str, int n)
{
    int i = 0, length = get_http_newline_length(str, n);
    while(length > 0 && i < n) {
        i += length;
        length = get_http_newline_length(str + i, n - i);
    }
    return i;
}

int find_from_char_set(const char* str, const char* char_set, int n)
{
    for(int i = 0; i < n; ++i) {
        char c = str[i];
        for(int j = 0; char_set[j]; ++j)
            if(c == char_set[j])
                return i;
    }
    return -1;
}

int iequals(const char* a, const char* b)
{
    unsigned size1 = strlen(a);
    if(strlen(b) != size1) return 0;
    for(unsigned i = 0; i < size1; ++i)
        if(tolower(a[i]) != tolower(b[i]))
            return 0;
    return 1;
}

int increment_array_pointer(char** array, int* len, int increment)
{
    if(increment >= *len) return -1;
    *array += increment;
    *len -= increment;
    return 0;
}
