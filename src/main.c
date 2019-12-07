#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <dirent.h>
#include <pthread.h>

#include "definitions.h"
#include "worker.h"
#include "files.h"
#include "utils.h"
#include "http_request.h"

// GLOBAL VARIABLES
worker* workers;
int sockfd;

int cur_connection;
pthread_mutex_t connection_lock;
pthread_cond_t cond_written, cond_read;
int cur_connection_written, cur_connection_read;

int init_pthread()
{
    int error = pthread_mutex_init(&connection_lock, NULL);
    if(error) {
        fprintf(stderr, "failed to create mutex, error code: %d", error);
        return 0;
    }
    error = pthread_cond_init(&cond_written, NULL);
    if(error) {
        fprintf(stderr, "railed to create write condition, error code: %d", error);
        return 0;
    }
    error = pthread_cond_init(&cond_read, NULL);
    if(error) {
        fprintf(stderr, "railed to create read condition, error code: %d", error);
        return 0;
    }
    return 1;
}

// port: input port to open connection on
// returns: the file descriptor for the socket, -1 if error occurred
int open_connection(u16 port)
{
    // make a listening socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        fprintf(stderr, "failed to create socket\n");
        return -1;
    }

    int reuse = 1; // allow for reuse of ports
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        fprintf(stderr, "failed to set socket options\n");
        return -1;
    }

    // start listening on the socket.
    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(sock, (struct sockaddr*)&address, sizeof(address)) == -1) {
        fprintf(stderr, "failed to bind socket\n");
        return -1;
    }
    if(listen(sock, SOMAXCONN) == -1) {
        fprintf(stderr, "failed to listen socket\n");
        return -1;
    }
    return sock;
}

void send_http_error_response(buffer* b, http_error error, int connection, const char* error_text)
{
    const char* headers = get_http_error_headers(error);
    const char* body = get_http_error_body(error);
    set_http_error_response(b, headers, body);
    if(write(connection, b->data, b->length) == -1) {
        fprintf(stderr, "failed to send response: %s", error_text);
    }
    close(connection);
}

void* handle_connection(void* args)
{
    worker* w = (worker*) args;
    s8 transfer_chunk[TRANSFER_CHUNK_SIZE];
    http_method method = 0;
    buffer content_type = make_buffer(8);
    buffer_insert_string(&content_type, "content.html");
    struct stat statbuf; // stores file info

    while(1) {
        w->req_buffer.length = w->resp_buffer.length = 0;

        // accept connection socket.
        pthread_mutex_lock(&connection_lock);
        while(!cur_connection_written) {
            pthread_cond_wait(&cond_written, &connection_lock);
        }
        cur_connection_written = 0;
        w->connection = cur_connection;
        cur_connection_read = 1;
        pthread_mutex_unlock(&connection_lock);
        pthread_cond_signal(&cond_read);

        // Read request stream into a buffer. Read in chunks of TRANSFER_CHUNK_SIZE.
        // Since we only accept GET and HEAD requests, just read up to first double newline.
        int valid_req = 0;
        while(1) {
            s64 received = recv(w->connection, transfer_chunk, TRANSFER_CHUNK_SIZE, 0);
            if(received == -1) {
                fprintf(stderr, "failed to receive data");
                break;
            }
            // See if we've found the end of the headers. Chunk in case double newline is split between chunks
            u64 idx = w->req_buffer.length > 3 ? w->req_buffer.length - 3 : 0;
            buffer_insert_bytes(&w->req_buffer, transfer_chunk, received);
            for(u64 i = idx; i < w->req_buffer.length; ++i) {
                if(find_http_header_end((const char*)(w->req_buffer.data + i), w->req_buffer.length - i)) {
                    valid_req = 1;
                    break;
                }
            }
            if(valid_req) break;
            else if(received < TRANSFER_CHUNK_SIZE || w->req_buffer.length > REQUEST_MAX_SIZE) break;
        }
        // parse request string into request struct
        if(!valid_req || parse_request(&w->req_buffer, &w->request) == -1) {
            send_http_error_response(&w->resp_buffer, BAD_REQUEST, w->connection, "bad request");
            continue;
        }

        method = get_http_method(w->request.method);
        if(method == UNSUPPORTED) {
            send_http_error_response(&w->resp_buffer, METHOD_NOT_SUPPORTED, w->connection, "method not supported");
            continue;
        }
        // only http 1.1 is supported
        if(!iequals((const char*)w->request.version.data, w->request.version.length, HTTP_1_1_VERSION)) {
            send_http_error_response(&w->resp_buffer, VERSION_NOT_SUPPORTED, w->connection, "only http 1.1 is supported");
            continue;
        }

        printf("%.*s %.*s handled by thread %d\n", (int)w->request.method.length, w->request.method.data, (int)w->request.path.length - 1, w->request.path.data + 1, w->id);

        if(get_file_stat(&w->request.path, &statbuf) == -1) {
            send_http_error_response(&w->resp_buffer, NOT_FOUND, w->connection, "requested file not found");
            continue;
        }
        // handle directory
        if((statbuf.st_mode & S_IFMT) == S_IFDIR) {
            if (w->request.path.data[w->request.path.length - 1] != '/') {
                buffer_insert_string(&w->request.path, "/");
            }
            // try to send index.html, keep track of length of original path in case this doesn't work.
            s64 base_length = w->request.path.length;
            buffer_insert_string(&w->request.path, "index.html");

            // otherwise send directory listing.
            if(get_file_stat(&w->request.path, &statbuf) == -1) {
                w->dir_list_buffer.length = w->dir_name_buffer.length = w->filename_buffer.length = 0;
                w->request.path.length = base_length;
                buffer_insert_string(&w->dir_list_buffer, "<html><body><h1>Current directory: ");
                buffer_insert_bytes(&w->dir_list_buffer, w->request.path.data + 1, w->request.path.length - 1); // skip '.'
                buffer_insert_string(&w->dir_list_buffer, "</h1><ul>\n");

                DIR* dir = open_dir(&w->request.path);
                if(!dir) {
                    send_http_error_response(&w->resp_buffer, NOT_FOUND, w->connection, "failed to open directory");
                    continue;
                }

                u64 num_dirs = 0, num_files = 0;
                struct dirent* entry = readdir(dir);
                while(entry) {
                    if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
                        entry = readdir(dir);
                        continue;
                    }
                    // reset to original path, i.e. get rid of name from last iteration of loop
                    w->request.path.length = base_length;
                    buffer_insert_string(&w->request.path, entry->d_name);
                    // names of each dir and file are kept in a single buffer, separated by null characters
                    if(entry->d_type == DT_DIR) {
                        buffer_insert_string(&w->dir_name_buffer, entry->d_name);
                        buffer_insert_char(&w->dir_name_buffer, '\0');
                        ++num_dirs;
                    } 
                    else if(entry->d_type == DT_REG) {
                        buffer_insert_string(&w->filename_buffer, entry->d_name);
                        buffer_insert_char(&w->filename_buffer, '\0');
                        ++num_files;
                    }
                    entry = readdir(dir);
                }

                char* dir_names[num_dirs], *filenames[num_files];
                dir_names[0] = (char*)w->dir_name_buffer.data, filenames[0] = (char*)w->filename_buffer.data;
                u64 cur_file = 1, cur_dir = 1; // indices

                char* current = (char*)w->dir_name_buffer.data;
                char* end = (char*)w->dir_name_buffer.data + w->dir_name_buffer.length;
                while(current != end && cur_dir < num_dirs) {
                    if(*current == '\0') dir_names[cur_dir++] = current + 1;
                    ++current;
                }
                current = (char*)w->filename_buffer.data;
                end = (char*)w->filename_buffer.data + w->filename_buffer.length;
                while(current != end && cur_file < num_files) {
                    if(*current == '\0') filenames[cur_file++] = current + 1;
                    ++current;
                }

                // sort the two lists of directories and filenames
                sort_filenames(dir_names, num_dirs);
                sort_filenames(filenames, num_files);

                // reset path again for display
                w->request.path.length = base_length;

                // list directories and files
                insert_filename_or_dirname(&w->dir_list_buffer, w->request.path, dir_names, num_dirs,  0);
                insert_filename_or_dirname(&w->dir_list_buffer, w->request.path, filenames, num_files, 1);
                buffer_insert_string(&w->dir_list_buffer, "</ul></body></html>\n");

                // prepare response headers
                insert_http_content_response_header(&w->resp_buffer, content_type, w->dir_list_buffer.length);

                // append listing if we got a GET request.
                if(method == GET) {
                    buffer_insert_bytes(&w->resp_buffer, w->dir_list_buffer.data, w->dir_list_buffer.length);
                }
                // send response.
                if(write(w->connection, w->resp_buffer.data, w->resp_buffer.length) == -1) {
                }

                close(w->connection);
                closedir(dir);
                continue;
            }
        }

        // try to send a file, should exist since it was created earlier
        int fd = open_file(&w->request.path, O_RDONLY);
        if(fd == -1) {
            send_http_error_response(&w->resp_buffer, NOT_FOUND, w->connection, "failed to open file");
            continue;
        }

        // create and send response headers
        insert_http_content_response_header(&w->resp_buffer, w->request.path, statbuf.st_size);
        if(write(w->connection, w->resp_buffer.data, w->resp_buffer.length) == -1) {
            fprintf(stderr, "failed to send response headers");
            close(w->connection);
            close(fd);
            continue;
        }

        // if we got a GET request, send file.
        if(method == GET) {
            u64 i = 0;
            while(i < (u64)statbuf.st_size) {
                u64 length = TRANSFER_CHUNK_SIZE;
                if(i + length > (u64)statbuf.st_size) length = statbuf.st_size - i;
                u64 num_read = read(fd, transfer_chunk, length);
                if(num_read > 0) i += num_read;
                else break;
                if(write(w->connection, transfer_chunk, length) == -1) {
                    fprintf(stderr, "failed to send response");
                    break;
                }
            }
        }
        // clean up.
        close(w->connection);
        close(fd);
    }
}

int main(int argc, char** argv)
{
    u16 port = 4950;
    if(argc > 1) {
        int arg_port = atoi(argv[1]);
        if(arg_port > 0) port = (u16)arg_port;
    }
    printf("spinning up server on port %d using %ld threads..\n", port, NUM_THREADS);

    sockfd = open_connection(port);
    if(sockfd < -1) return -1;

    if(!init_pthread()) return -1;
    workers = make_workers(NUM_THREADS, handle_connection);
    if(!workers) return -1;

    printf("server is now listening.. \n");
    while(1) {
        int connection = accept(sockfd, 0, 0);
        if(connection == -1) {
            fprintf(stderr, "connection failed\n");
            continue;
        }

        // pass accepted socket connection(work) to worker.
        pthread_mutex_lock(&connection_lock);
        cur_connection = connection;
        cur_connection_written = 1;
        pthread_mutex_unlock(&connection_lock);
        pthread_cond_signal(&cond_written);

        pthread_mutex_lock(&connection_lock);
        while(!cur_connection_read) {
            pthread_cond_wait(&cond_read, &connection_lock);
        }
        cur_connection_read = 0;
        pthread_mutex_unlock(&connection_lock);
    }
    free_workers(workers, NUM_THREADS);

    return 0;
}
