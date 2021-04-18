// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "sockettome.h"

void *processClientThread(void *c) {
    printf("Accepted Client\n");
    FILE **connection;
    char buf[1000];

    connection = (FILE **) c;
    while (fgets(buf, 1000, connection[0]) != NULL) {
        printf("Read: %s", buf);
        fflush(stdout);
        fputs(buf, connection[1]);
        fflush(connection[1]);
    }
    printf("Socket closed\n");
    fflush(stdout);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: ./chat_server <port> <room names>\n");
        exit(1);
    }

    int port = atoi(argv[1]);
    printf("Read in port: %d\n", port);

    if (port < 8000) {
        fprintf(stderr, "Port must be > 8000\n");
        exit(1);
    }

    int sock = serve_socket(port);
    // int fd = accept_connection(sock);
    FILE *fin, *fout;
    FILE *stdin_to_socket[2];
    FILE *socket_to_stdout[2];
    pthread_t tid;
    int i = 0;

    while (1) {
        int fd = accept_connection(sock);
        fin = fdopen(fd, "r");
        fout = fdopen(fd, "w");
        
        /* Create arrays of FILE *'s for process_connection. */

        stdin_to_socket[0] = stdin;
        stdin_to_socket[1] = fout;

        socket_to_stdout[0] = fin;
        socket_to_stdout[1] = stdout;
        if (pthread_create(&tid, NULL, processClientThread, socket_to_stdout) != 0) {
            perror("pthread_create");
            exit(0);
        }
    }

    return 0;
}