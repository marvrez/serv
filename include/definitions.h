#ifndef PARAMETERS_H
#define PARAMETERS_H

#define SERV_VERSION "1.0"

#include <inttypes.h>
#include <unistd.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#ifdef _SC_NPROCESSORS_ONLN
#define NUM_THREADS sysconf(_SC_NPROCESSORS_ONLN)
#else
#define NUM_THREADS 4
#endif

typedef enum {
    UNSUPPORTED=-1,
    GET=1,
    HEAD,
} http_method;

typedef enum {
    UNKNOWN_ERROR=-1,
    BAD_REQUEST=0,
    NOT_FOUND,
    METHOD_NOT_SUPPORTED,
    VERSION_NOT_SUPPORTED,
} http_error;

#define HTTP_1_1_VERSION "HTTP/1.1"
#define HTTP_OK_HEADER "HTTP/1.1 200 OK\r\n"
#define HTTP_CACHE_HEADERS "Server: serv/" SERV_VERSION "\r\nCache-control: no-cache, no-store, must-revalidate\r\nExpires: 0\r\nPragma: no-cache\r\n"
#define HTTP_CONTENT_TYPE_KEY "Content-Type: "
#define HTTP_CONTENT_LENGTH_KEY "Content-Length: "
#define HTTP_DATE_KEY "Date: "
#define HTTP_NEWLINE "\r\n"
#define HTTP_END_HEADER HTTP_NEWLINE HTTP_NEWLINE

// error headers and bodies
#define BAD_REQUEST_HEADERS "HTTP/1.1 400 BAD REQUEST\r\nServer: serv/" SERV_VERSION "\r\nContent-Type: text/html\r\nContent-Length: 59\r\n"
#define BAD_REQUEST_BODY "<html><body>\n<h1>Invalid HTTP request!</h1>\n</body></html>\n"
#define NOT_FOUND_HEADERS "HTTP/1.1 404 NOT FOUND\r\nServer: serv/" SERV_VERSION "\r\nContent-Type: text/html\r\nContent-Length: 53\r\n"
#define NOT_FOUND_BODY "<html><body>\n<h1>File not found!</h1>\n</body></html>\n"
#define METHOD_NOT_SUPPORTED_HEADERS "HTTP/1.1 501 NOT IMPLEMENTED\r\nServer: serv/" SERV_VERSION "\r\nContent-Type: text/html\r\nContent-Length: 55\r\n"
#define METHOD_NOT_SUPPORTED_BODY "<html><body>\n<h1>Method not supported!</h1>\n</body></html>\n"
#define VERSION_NOT_SUPPORTED_HEADERS "HTTP/1.1 505 VERSION NOT SUPPORTED\r\nServer: serv/" SERV_VERSION "\r\nContent-Type: text/html\r\nContent-Length: 63\r\n"
#define VERSION_NOT_SUPPORTED_BODY "<html><body>\n<h1>HTTP version must be 1.1!</h1>\n</body></html>\n"

// 4kB
#define TRANSFER_CHUNK_SIZE 1 << 15
#define REQUEST_MAX_SIZE (4*TRANSFER_CHUNK_SIZE)

#define BYTESET_TOKEN_END " \t\r\n"
#define BYTESET_PATH_END "?#" BYTESET_TOKEN_END
#define BYTESET_HEADER_KEY_END ":" BYTESET_TOKEN_END

#endif
