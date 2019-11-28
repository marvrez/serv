#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "definitions.h"
#include "utils.h"

// port: input port to open connection on
// returns: the file descriptor for the socket, -1 if error occurred
int open_connection(u16 port)
{
    // make a listening socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        fprintf(stderr, "failed to create socket\n");
        return -1;
    }

    int reuse = 1; // allow for reuse of ports
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        fprintf(stderr, "failed to set socket options\n");
        return -1;
    }

    // start listening on the socket.
    struct sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        fprintf(stderr, "failed to bind socket\n");
        return -1;
    }
    if(listen(sockfd, SOMAXCONN) == -1) {
        fprintf(stderr, "failed to listen socket\n");
        return -1;
    }
    return sockfd;
}

int main(int argc, char** argv)
{
    u16 port = 4950;
    if(argc > 1) {
        int arg_port = atoi(argv[1]);
        if(arg_port > 0) port = (u16)arg_port;
    }
    printf("spinning up server on port %d using %ld threads..\n", port, NUM_THREADS);

    int sockfd = open_connection(port);
    if(sockfd < -1) return 1;

    printf("server is now listening.. \n");
    while(1) {
        int connection = accept(sockfd, 0, 0);
        if(connection == -1) {
            fprintf(stderr, "connection failed\n");
            continue;
        }
    }

    return 0;
}
