#include "http_request.h"
#include "utils.h"

#include <stdlib.h>

static inline int decode_buffer(buffer* b)
{
    s8* path = b->data;
    u64 length = b->length, read_idx = 0, write_idx = 0;
    while(read_idx < length) {
        if(path[read_idx] != '%') {
            path[write_idx++] = path[read_idx++];
            continue;
        }
        if(read_idx + 2 >= length) return 0;
        s8 c = (s8)strtol((char*)(path + read_idx + 1), NULL, 16);
        if(c > 0) {
            path[write_idx++] = c;
            read_idx += 3;
        } 
        else return 0;
    }
    b->length = write_idx;
    return 1;
}

static inline void remove_dot_segments(buffer* b)
{
    // skip "./"
    s8* path = b->data + 2, c1, c2, c3;
    u64 length = b->length - 2, read_idx = 0, write_idx = 0;
    while(read_idx < length) {
        c1 = path[read_idx];
        // only interested in segments beginning with '.'
        if(c1 != '.' || (read_idx > 0 && path[read_idx - 1] != '/')) {
            path[write_idx++] = path[read_idx++];
            continue;
        }
        if(read_idx + 1 == length) break;
        c2 = path[read_idx + 1];

        if(c2 == '/') read_idx += 2;
        else if(c2 == '.') {
            if(read_idx + 2 == length) break;
            c3 = path[read_idx + 2];
            if(c3 == '/') {
                read_idx += 3;
                if(write_idx > 0) {
                    --write_idx;
                    while(write_idx > 0 && path[write_idx - 1] != '/') --write_idx;
                }
            }
            else {
                path[write_idx] = path[read_idx];
                path[write_idx + 1] = path[read_idx + 1];
                read_idx += 2, write_idx += 2;
            }
        } 
        else path[write_idx++] = path[read_idx++];
    }
    b->length = write_idx + 2;
}

s8 parse_request(const buffer* b, http_request* req)
{
    req->method.length = req->path.length = req->version.length = 0;

    char* data = (char*)b->data;
    u64 data_length = b->length;

    // skip leading newlines
    s64 idx = ltrim_http_newline(data, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    // get method
    idx = ltrim_space(data, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    idx = find_from_char_set(data, BYTESET_TOKEN_END, data_length);
    if(idx == -1) return -1;
    buffer_insert_bytes(&req->method, (s8*)data, idx);
    data += idx, data_length -= idx;

    // get path
    buffer_insert_char(&req->path, '.');

    idx = ltrim_space(data, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    idx = find_from_char_set(data, BYTESET_PATH_END, data_length);
    if(idx == -1) return -1;
    buffer_insert_bytes(&req->path, (s8*)data, idx);
    data += idx, data_length -= idx;

    if(!decode_buffer(&req->path)) return -1;
    remove_dot_segments(&req->path);

    // skip over query (?) and fragment (#) parts, if they exist.
    idx = find_from_char_set(data, BYTESET_TOKEN_END, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    // http version string
    idx = ltrim_space(data, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    idx = find_from_char_set(data, BYTESET_TOKEN_END, data_length);
    if(idx == -1) return -1;
    buffer_insert_bytes(&req->version, (s8*)data, idx);
    data += idx, data_length -= idx;

    idx = ltrim_space(data, data_length);
    if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

    if(!find_http_newline(data, data_length)) return -1;

    // find "Host" header. Required to respond with 400 if not found (RFC 7230, 5.4)
    s8 host = 0;
    while(!host && (u64)idx < data_length) {
        s64 idx = ltrim_http_newline(data, data_length);
        if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;
        idx = ltrim_space(data, data_length);
        if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

        // get header key
        idx = find_from_char_set(data, BYTESET_HEADER_KEY_END, data_length);
        if(idx == -1) return -1;

        // check if it's the host header
        if(iequals(data, idx, "Host")) host = 1;
        if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

        // ":" has to come immediately after header key (RFC 7230, 3.2.4)
        if(*data != ':') return -1;

        // don't care what the header value is.
        idx = find_from_char_set(data, HTTP_NEWLINE, data_length);
        if(idx == -1) return -1;
        if(increment_array_pointer(&data, &data_length, idx) == -1) return -1;

        // make sure it has a proper line ending.
        if(!find_http_newline(data, data_length)) return -1;
        if(host || find_http_header_end(data, data_length)) break;
    }

    // bad request if "Host" header is not found
    if(!host) return -1;

    return 0;
}
