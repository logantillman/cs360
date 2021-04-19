// Author: Logan Tillman

// ./chat_server 8990 Bridge Baseball Politics Video-Games Art Music Movies Food Woodworking American-Idol

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "sockettome.h"
#include "dllist.h"
#include "jrb.h"

typedef struct {
    char *roomName;
    Dllist clients;
    Dllist messages;
    pthread_t *tid;
    pthread_mutex_t *userLock;
} Room_Info;

typedef struct {
    char *chatName;
    Room_Info *chatRoom;
    FILE *fin;
    FILE *fout;
    int fd;
} Thread_Info;

typedef struct {
    Thread_Info *client;
    Dllist chatRooms;
} Chat_Data;

void *clientThread(void *chatDataP);

void *serverThread(void *room);

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

    int i;
    JRB roomList = make_jrb();

    for (i = 2; i < argc; i++) {
        Room_Info *chatRoom = (Room_Info *) malloc(sizeof(Room_Info));
        chatRoom->roomName = argv[i];
        chatRoom->clients = new_dllist();
        chatRoom->messages = new_dllist();
        chatRoom->userLock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
        chatRoom->tid = (pthread_t *) malloc(sizeof(pthread_t));

        pthread_mutex_init(chatRoom->userLock, NULL);
        pthread_create(chatRoom->tid, NULL, serverThread, chatRoom);
        jrb_insert_str(roomList, chatRoom->roomName, new_jval_v((void *) chatRoom));
    }

    int socket = serve_socket(port);

    while (1) {
        int fd = accept_connection(socket);
        if (fd > 1) {
            printf("Accepted client\n");
            Thread_Info *client = (Thread_Info *) malloc(sizeof(Thread_Info));
            Chat_Data *chatData = (Chat_Data *) malloc(sizeof(Chat_Data));

            client->fd = fd;
            client->fin = fdopen(fd, "r");
            client->fout = fdopen(fd, "w");

            chatData->client = client;
            chatData->chatRooms = roomList;

            pthread_t tid;
            pthread_create(&tid, NULL, clientThread, chatData);
        }
    }
    return 0;
}

void *clientThread(void *chatDataP) {
    Chat_Data *chatData = (Chat_Data *) chatDataP;
    Thread_Info *client = chatData->client;
    char *buf = (char *) malloc(1000);
    char *new = (char *) malloc(1000);
    int i;

    client->chatName = NULL;
    client->chatRoom = NULL;

    if (client->fin == NULL) {
        perror("fin error");
        exit(1);
    }

    if (client->fout == NULL) {
        perror("fout error");
        exit(1);
    }

    fprintf(client->fout, "Chat Rooms:\n\n");

    JRB tmpRooms;
    JRB chatRooms = chatData->chatRooms;
    Dllist tmpClients;

    jrb_traverse(tmpRooms, chatRooms) {
        Room_Info *chatRoom = (Room_Info *) tmpRooms->val.v;

        fprintf(client->fout, "%s:", chatRoom->roomName);

        dll_traverse(tmpClients, chatRoom->clients) {
            Thread_Info *tmpClient = (Thread_Info *) tmpClients->val.v;
            // if (tmpClient != NULL && tmpClient->chatName != NULL) {
                fprintf(client->fout, " %s", tmpClient->chatName);
            // }
        }

        fprintf(client->fout, "\n");
        fflush(client->fout);
    }

    fprintf(client->fout, "\n");
    fflush(client->fout);

    while (1) {
        fprintf(client->fout, "Enter your chat name (no spaces):\n");
        fflush(client->fout);

        if (fgets(buf, 1000, client->fin) == NULL) {
            close(client->fd);
            fclose(client->fin);
            fclose(client->fout);
            free(client);

            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        buf[strlen(buf) - 1] = '\0';
        int isEmpty = 0;

        for (i = 0; i < strlen(buf); i++) {
            if (isblank(buf[i])) {
                isEmpty = 1;
                break;
            }
        }
        if (!isEmpty) {
            client->chatName = buf;
            break;
        }
    }

    while (1) {
        fprintf(client->fout, "Enter chat room:\n");
        fflush(client->fout);
        buf = (char *) malloc(1000);

        if (fgets(buf, 1000, client->fin) == NULL) {
            close(client->fd);
            fclose(client->fin);
            fclose(client->fout);

            free(client->chatName);
            free(client);

            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        buf[strlen(buf) - 1] = '\0';

        jrb_traverse(tmpRooms, chatRooms) {
            Room_Info *chatRoom = (Room_Info *) tmpRooms->val.v;
            if (strcmp(chatRoom->roomName, buf) == 0) {
                client->chatRoom = chatRoom;
                dll_append(client->chatRoom->clients, new_jval_v((void *) client));
                break;
            }
        }

        if (client->chatRoom != NULL) {
            break;
        }
    }

    char *message = (char *) malloc(1000);
    sprintf(message, "%s has joined\n", client->chatName);

    pthread_mutex_lock(client->chatRoom->userLock);
    dll_append(client->chatRoom->messages, new_jval_s(message)); // Maybe mem leak here
    pthread_mutex_unlock(client->chatRoom->userLock);

    message = (char *) malloc(1000);

    while (fgets(message, 1000, client->fin) != NULL) {
        char *new = (char *) malloc(1000);
        sprintf(new, "%s: %s\0", client->chatName, message);
        free(message);

        pthread_mutex_lock(client->chatRoom->userLock);
        dll_append(client->chatRoom->messages, new_jval_s(new));
        pthread_mutex_unlock(client->chatRoom->userLock);

        message = (char *) malloc(1000);
    }

    dll_traverse(tmpClients, client->chatRoom->clients) {
        Thread_Info *tmpClient = (Thread_Info *) tmpClients->val.v;
        if (strcmp(tmpClient->chatName, client->chatName) == 0) {
            dll_delete_node(tmpClients);
            break;
        }
    }
    close(client->fd);
    fclose(client->fin);
    fclose(client->fout);

    sprintf(new, "%s has left\n", client->chatName);

    pthread_mutex_lock(client->chatRoom->userLock);
    dll_append(client->chatRoom->messages, new_jval_s(new));
    pthread_mutex_unlock(client->chatRoom->userLock);

    free(client->chatName);
    free(client);
    
    pthread_detach(pthread_self());
    return NULL;
}

void *serverThread(void *room) {
    Room_Info *chatRoom = (Room_Info *) room;

    Dllist rTemp;
    Dllist mTemp;
    Dllist temp;

    while (1) {
        if (!dll_empty(chatRoom->messages)) {
            for (mTemp = (chatRoom->messages)->flink; mTemp != (chatRoom->messages);) {
                if (mTemp->flink != mTemp && mTemp != NULL) {
                    jrb_traverse(rTemp, chatRoom->clients) {
                        if (rTemp != NULL) {
                            fprintf(((Thread_Info *)(rTemp->val.v))->fout, "%s", (chatRoom->messages)->flink->val.s);
                            fflush(((Thread_Info *)(rTemp->val.v))->fout);
                        }
                    }

                    temp = mTemp->flink;

                    if (mTemp->val.s != NULL && mTemp != NULL) {
                        pthread_mutex_lock(chatRoom->userLock);
                        free(mTemp->val.s);
                        dll_delete_node(mTemp);
                        pthread_mutex_unlock(chatRoom->userLock);
                    }

                    if (chatRoom->messages == temp) {
                        break;
                    }

                    mTemp = temp;
                }
                else {
                    break;
                }
            }
        }
    }
}