#include "files.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int open_file(buffer* b, int flags)
{
    buffer_ensure_null(b);
    return open((const char*)b->data, flags);
}

DIR* open_dir(buffer* b)
{
    buffer_ensure_null(b);
    return opendir((const char*)b->data);
}

int get_file_stat(buffer* b, struct stat* statbuf)
{
    buffer_ensure_null(b);
    return stat((const char*)b->data, statbuf);
}

char* get_content_type(buffer* filename)
{
    s64 offset = filename->length - 1;
    while(offset > 0 && filename->data[offset] != '.') --offset;
    if(offset == 0) return "application/octet-stream";

    const char* str = (const char*)(filename->data + offset);
    u64 length = filename->length - offset;

    // text
    if(iequals(str, length, ".html") || iequals(str, length, ".htm")) return "text/html";
    if(iequals(str, length, ".js")) return "application/javascript";
    if(iequals(str, length, ".css")) return "text/css";
    if(iequals(str, length, ".xml")) return "text/xml";
    if(iequals(str, length, ".json")) return "application/json";
    if(iequals(str, length, ".txt")) return "text/plain";

    // images
    if(iequals(str, length, ".jpeg") || iequals(str, length, ".jpg")) return "image/jpeg";
    if(iequals(str, length, ".png")) return "image/png";
    if(iequals(str, length, ".gif")) return "image/gif";
    if(iequals(str, length, ".bmp")) return "image/bmp";
    if(iequals(str, length, ".svg")) return "image/svg+xml";

    // video
    if(iequals(str, length, ".ogv")) return "video/ogg";
    if(iequals(str, length, ".mp4")) return "video/mp4";
    if(iequals(str, length, ".mpg") || iequals(str, length, ".mpeg")) return "video/mpeg";
    if(iequals(str, length, ".mov")) return "video/quicktime";

    // audio
    if(iequals(str, length, ".ogg")) return "application/ogg";
    if(iequals(str, length, ".oga")) return "audio/ogg";
    if(iequals(str, length, ".mp3")) return "audio/mpeg"; 
    if(iequals(str, length, ".wav")) return "audio/wav";

    return "application/octet-stream";
}

static inline int cstring_cmp(const void* a, const void* b)
{
    const char** ia = (const char**)a;
    const char** ib = (const char**)b;
    return strcmp(*ia, *ib);
}

void sort_filenames(char** filenames, int n)
{
    qsort(filenames, n, sizeof(char*), cstring_cmp);
}
