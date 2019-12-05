#include "utils.h"
#include "files.h"

#include <string.h>
#include <ctype.h>

int find_http_newline(const char* str, int n)
{
    if(n < 1) return 0;
    if(str[0] == '\n') return 1;
    if(n > 1 && str[0] == '\r' && str[1] == '\n') return 2;
    return 0;
}

int find_http_header_end(const char* str, int n)
{
    if(n < 2) return 0;
    int i = find_http_newline(str, n);
    if(i == 0) return 0;
    int j = find_http_newline(str + i, n - i);
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
    int i = 0, length = find_http_newline(str, n);
    while(length > 0 && i < n) {
        i += length;
        length = find_http_newline(str + i, n - i);
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

int iequals(const char* a, unsigned len_a, const char* b)
{
    if(strlen(b) != len_a) return 0;
    for(unsigned i = 0; i < len_a; ++i) {
        if(tolower(a[i]) != tolower(b[i]))
            return 0;
    }
    return 1;
}

int increment_array_pointer(char** array, u64* len, s64 increment)
{
    if((u64)increment >= *len) return -1;
    *array += increment;
    *len -= increment;
    return 0;
}

void set_http_error_response(buffer* b, const char* headers, const char* body)
{
    b->length = 0;
    buffer_insert_string(b, headers);
    buffer_insert_string(b, HTTP_DATE_KEY);
    buffer_insert_current_date(b);
    buffer_insert_string(b, HTTP_END_HEADER);
    buffer_insert_string(b, body);
}

const char* get_http_error_headers(http_error error)
{
    switch(error) {
        case BAD_REQUEST:
            return BAD_REQUEST_HEADERS;
        case NOT_FOUND:
            return NOT_FOUND_HEADERS;
        case METHOD_NOT_SUPPORTED:
            return METHOD_NOT_SUPPORTED_HEADERS;
        case VERSION_NOT_SUPPORTED:
            return VERSION_NOT_SUPPORTED_HEADERS;
        default:
            break;
    }
    return "unknown error!";
}

const char* get_http_error_body(http_error error)
{
    switch(error) {
        case BAD_REQUEST:
            return BAD_REQUEST_BODY;
        case NOT_FOUND:
            return NOT_FOUND_BODY;
        case METHOD_NOT_SUPPORTED:
            return METHOD_NOT_SUPPORTED_BODY;
        case VERSION_NOT_SUPPORTED:
            return VERSION_NOT_SUPPORTED_BODY;
        default:
            break;
    }
    return "unknown error!";
}

http_method get_http_method(buffer b)
{
    if(iequals((char*)b.data, b.length, "GET")) return GET;
    if(iequals((char*)b.data, b.length, "HEAD")) return HEAD;
    return UNSUPPORTED;
}
