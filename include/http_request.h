#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "buffer.h"
#include "definitions.h"

typedef struct {
    buffer method;
    buffer path;
    buffer version;
} http_request;

s8 parse_request(const buffer* b, http_request* req);

#endif
