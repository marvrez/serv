#include "worker.h"

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

worker* make_workers(int num_workers, void* (*work_handler)(void*))
{
    worker* workers = malloc(num_workers*sizeof(worker));
    for(int i = 0; i < num_workers; ++i) {
        worker* w = workers + i;
        w->id = i;
        w->request.method = make_buffer(16);
        w->request.path = make_buffer(1024);
        w->request.version = make_buffer(16);
        w->req_buffer = make_buffer(2048);
        w->resp_buffer = make_buffer(1024);
        w->dir_list_buffer = make_buffer(512);
        w->dir_name_buffer = make_buffer(512);
        w->filename_buffer = make_buffer(512);
        int error = pthread_create(&w->thread, NULL, work_handler, &workers[i]);
        if(error) {
            fprintf(stderr, "failed to create thread. Error code: %d", error);
            return NULL;
        }
    }
    return workers;
}

void free_workers(worker* workers, int num_workers)
{
    for(int i = 0; i < num_workers; ++i) {
        pthread_cancel(workers[i].thread);
        free_buffer(&workers[i].req_buffer);
        free_buffer(&workers[i].resp_buffer);
        free_buffer(&workers[i].request.method);
        free_buffer(&workers[i].request.path);
        free_buffer(&workers[i].request.version);
        free_buffer(&workers[i].dir_list_buffer);
        free_buffer(&workers[i].dir_name_buffer);
        free_buffer(&workers[i].filename_buffer);
        close(workers[i].connection);
    }
    free(workers);
}
