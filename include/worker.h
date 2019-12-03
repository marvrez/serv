#ifndef WORKER_H
#define WORKER_H

#include "http_request.h"
#include "buffer.h"

#include <pthread.h>

typedef struct {
    pthread_t thread;
    http_request request;
    buffer req_buffer;
    buffer resp_buffer;
    buffer dir_list_buffer;
    buffer dir_name_buffer;
    buffer filename_buffer;
    int id;
    int connection;
} worker;

worker* make_workers(int num_workers, void* (*work_handler)(void*));
void free_workers(worker* workers, int num_workers);

#endif
