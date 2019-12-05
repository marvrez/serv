#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

static const char* DAYS[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char* MONTHS[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

buffer make_buffer(int capacity)
{
    buffer b;
    b.data = malloc(capacity);
    b.length = 0;
    if(!b.data) fprintf(stderr, "couldn't allocate buffer, out of memory\n");
    b.capacity = capacity > 0 ? capacity : 1;
    return b;
}

void free_buffer(buffer* b)
{
    if(!b->data) return;
    free(b->data);
    b->data = 0;
    b->length = b->capacity = 0;
}

void buffer_realloc(buffer* b, unsigned n)
{
    if(n <= b->capacity) return;
    u64 capacity = b->capacity;
    s8* new_data;
    while(capacity < n) capacity <<= 1;
    new_data = realloc(b->data, capacity);
    if(!new_data) fprintf(stderr, "couldn't reallocate buffer, out of memory\n");
    b->data = new_data, b->capacity = capacity;
}

void buffer_ensure_null(buffer* b)
{
    if(b->data[b->length-1] == '\0') return;
    buffer_realloc(b, b->length + 1);
    b->data[b->length] = '\0';
}

void buffer_insert_bytes(buffer* b, s8* bytes, unsigned n)
{
    buffer_realloc(b, b->length + n);
    memcpy(b->data + b->length, bytes, n);
    b->length += n;
}

void buffer_insert_char(buffer* b, char c)
{
    buffer_insert_bytes(b, (s8*)&c, 1);
}

void buffer_insert_string(buffer* b, const char* str)
{
    unsigned n = strlen(str);
    buffer_insert_bytes(b, (s8*)str, n);
}

void buffer_insert_uint(buffer* b, unsigned int num)
{
    int length = ceil(log10(num)) + 1;
    char str[length];
    sprintf(str, "%u", num);
    buffer_insert_string(b, str);
}

void buffer_insert_current_date(buffer* b)
{
    time_t t = time(0);
    struct tm date;
    gmtime_r(&t, &date);

    buffer_realloc(b, b->length + 29);

    char str[32];
    sprintf(str, "%s, %u %s %u ", DAYS[date.tm_wday], date.tm_mday, MONTHS[date.tm_mon], date.tm_year+1900);
    buffer_insert_string(b, str);

    if(date.tm_hour < 10) buffer_insert_char(b, '0');
    buffer_insert_uint(b, date.tm_hour);
    buffer_insert_char(b, ':');

    if(date.tm_min < 10) buffer_insert_char(b, '0');
    buffer_insert_uint(b, date.tm_min);
    buffer_insert_char(b, ':');

    if(date.tm_sec < 10) buffer_insert_char(b, '0');
    buffer_insert_uint(b, date.tm_sec);

    buffer_insert_string(b, " GMT");
}
