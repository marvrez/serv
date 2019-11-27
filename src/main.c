#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "definitions.h"

// u16 port: input port to open connection on
// returns: the file descriptor for the socket, -1 if error occurred
s32 open_connection(u16 port)
{
    // make a listening socket
    s32 sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        fprintf(stderr, "failed to create socket\n");
        return -1;
    }

    s32 reuse = 1; // allow for reuse of ports
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

int main(int argc, char** argv)
{
    u16 port = 4950;
    if(argc > 1) {
        int arg_port = atoi(argv[1]);
        if(arg_port > 0) port = (u16)arg_port;
    }
    printf("spinning up server on port %d using %ld threads..\n", port, NUM_THREADS);

    s32 sock = open_connection(port);
    if(sock < -1) return 1;

    printf("server is now listening.. \n");
    while(1) {
        s32 connection = accept(sock, 0, 0);
        if(connection == -1) {
            fprintf(stderr, "connection failed\n");
            continue;
        }
    }

    return 0;
}
