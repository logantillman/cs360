// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "sockettome.h"
#include "dllist.h"
#include "jrb.h"

/* Struct for storing the chat room information */
typedef struct {
    char *roomName;
    Dllist clients;
    Dllist messages;
    pthread_t *tid;
    pthread_mutex_t *lock;
} Room_Info;

/* Struct for storing the client information */
typedef struct {
    char *chatName;
    Room_Info *chatRoom;
    FILE *fin;
    FILE *fout;
    int fd;
} Thread_Info;

/* Struct for storing chat data */
typedef struct {
    Thread_Info *client;
    Dllist chatRooms;
} Chat_Data;

/* Method that handles the threading for each client */
void *clientThread(void *chatDataP);

/* Method that handles the threading for each chat room */
void *serverThread(void *room);

int main(int argc, char **argv) {

    /* Error checking the command line arguments */
    if (argc < 3) {
        fprintf(stderr, "Usage: ./chat_server <port> <room names>\n");
        exit(1);
    }

    /* Reading in the port and error checking it */
    int port = atoi(argv[1]);
    if (port < 8000) {
        fprintf(stderr, "Port must be > 8000\n");
        exit(1);
    }

    int i;
    JRB roomList = make_jrb();

    /* Initializing each chat room struct and creating a thread for it */
    for (i = 2; i < argc; i++) {
        Room_Info *chatRoom = (Room_Info *) malloc(sizeof(Room_Info));
        chatRoom->roomName = argv[i];
        chatRoom->clients = new_dllist();
        chatRoom->messages = new_dllist();
        chatRoom->tid = (pthread_t *) malloc(sizeof(pthread_t));
        chatRoom->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

        jrb_insert_str(roomList, chatRoom->roomName, new_jval_v((void *) chatRoom));
        pthread_mutex_init(chatRoom->lock, NULL);
        pthread_create(chatRoom->tid, NULL, serverThread, chatRoom);
    }

    /* Serving the socket */
    int socket = serve_socket(port);

    /* Waiting for clients to connect */
    while (1) {
        int fd = accept_connection(socket);

        /* If the client successfully connects */
        if (fd > 1) {

            /* Initialize our client and the chatData objects */
            Thread_Info *client = (Thread_Info *) malloc(sizeof(Thread_Info));
            Chat_Data *chatData = (Chat_Data *) malloc(sizeof(Chat_Data));

            /* Opening the read and write fds for the client */
            client->fd = fd;
            client->fin = fdopen(fd, "r");
            client->fout = fdopen(fd, "w");

            /* Initializing the chatName and chatRoom fields for the client */
            client->chatName = NULL;
            client->chatRoom = NULL;

            chatData->client = client;
            chatData->chatRooms = (Dllist) roomList;

            /* Creating the client thread */
            pthread_t tid;
            pthread_create(&tid, NULL, clientThread, chatData);
        }
    }
    return 0;
}

/* Method that handles the threading for each client */
void *clientThread(void *chatDataP) {

    /* Converting the void pointer parameter back into a chat data pointer then extracting the client */
    Chat_Data *chatData = (Chat_Data *) chatDataP;
    Thread_Info *client = chatData->client;
    int i;

    if (client->fin == NULL) {
        perror("fin error");
        exit(1);
    }

    if (client->fout == NULL) {
        perror("fout error");
        exit(1);
    }

    /* Printing out the chat rooms */
    fprintf(client->fout, "Chat Rooms:\n\n");

    /* Converting the dllist to a JRB tree so the rooms print in alphabetical order */
    JRB chatRooms = (JRB) chatData->chatRooms;
    JRB tmpRooms;
    Dllist tmpClients;
    jrb_traverse(tmpRooms, chatRooms) {

        /* Printing out the room name */
        Room_Info *chatRoom = (Room_Info *) tmpRooms->val.v;
        fprintf(client->fout, "%s:", chatRoom->roomName);

        /* Printing out each client in the room */
        dll_traverse(tmpClients, chatRoom->clients) {
            Thread_Info *tmpClient = (Thread_Info *) tmpClients->val.v;
            fprintf(client->fout, " %s", tmpClient->chatName);
        }

        fprintf(client->fout, "\n");
        fflush(client->fout);
    }

    fprintf(client->fout, "\n");
    fflush(client->fout);

    /* Prompting the user to enter a chat name */
    while (1) {
        fprintf(client->fout, "Enter your chat name (no spaces):\n");
        fflush(client->fout);

        char *nameBuffer = (char *) malloc(100 * sizeof(char));

        /* If there's an error reading in the name, we close the fds and detach the thread */
        if (fgets(nameBuffer, 100, client->fin) == NULL) {
            close(client->fd);
            fclose(client->fin);
            fclose(client->fout);
            free(client);

            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        /* Setting the last character to the null char to override the new line character that was set by fgets */
        nameBuffer[strlen(nameBuffer) - 1] = '\0';
        int isSpace = 0;

        /* Checking to make sure there is no white space in the chat name */
        for (i = 0; i < strlen(nameBuffer); i++) {
            if (isblank(nameBuffer[i])) {
                isSpace = 1;
                break;
            }
        }

        /* As long as there is no white space in the name, we add the chat name to the client object */
        if (!isSpace) {
            client->chatName = (char *) strdup(nameBuffer);
            free(nameBuffer);
            break;
        }

        /* Freeing the buffer */
        free(nameBuffer);
    }

    /* Prompting the user to enter a chat room */
    while (1) {
        fprintf(client->fout, "Enter chat room:\n");
        fflush(client->fout);

        char *roomBuffer = (char *) malloc(100 * sizeof(char));

        /* If there's an error reading the room name, close our fds, free the memory, and detach the thread */
        if (fgets(roomBuffer, 100, client->fin) == NULL) {
            close(client->fd);
            fclose(client->fin);
            fclose(client->fout);

            free(client->chatName);
            free(client);
            free(roomBuffer);

            pthread_detach(pthread_self());
            pthread_exit(NULL);
        }

        /* Setting the last character to the null char to replace the new line char */
        roomBuffer[strlen(roomBuffer) - 1] = '\0';

        /* Traversing all of the chat rooms and comparing the name to make sure a valid room was entered */
        jrb_traverse(tmpRooms, chatRooms) {
            Room_Info *chatRoom = (Room_Info *) tmpRooms->val.v;

            /* If a valid room was entered, set the client's chat room property and add the client to the chat room's client list */
            if (strcmp(chatRoom->roomName, roomBuffer) == 0) {
                client->chatRoom = chatRoom;
                dll_append(client->chatRoom->clients, new_jval_v((void *) client));
                break;
            }
        }

        /* If the chat room was successfully set, we can break out of this loop, otherwise we prompt the client again */
        if (client->chatRoom != NULL) {
            free(roomBuffer);
            break;
        }

        free(roomBuffer);
    }

    /* Constructing the join message */
    char *message = (char *) malloc(500 * sizeof(char));
    sprintf(message, "%s has joined\n", client->chatName);

    /* Adding the join message to the messages dllist for the chat room, making sure to lock and unlock the mutex accordingly */
    pthread_mutex_lock(client->chatRoom->lock);
    dll_append(client->chatRoom->messages, new_jval_s(message));
    pthread_mutex_unlock(client->chatRoom->lock);

    /* Reading in chat messages from the client */
    message = (char *) malloc(500 * sizeof(char));
    while (fgets(message, 500, client->fin) != NULL) {
        char *completeMessage = (char *) malloc(500 * sizeof(char));
        sprintf(completeMessage, "%s: %s\0", client->chatName, message);
        free(message);

        /* Adding the chat message to the dllist, again making sure to lock and unlock the mutex */
        pthread_mutex_lock(client->chatRoom->lock);
        dll_append(client->chatRoom->messages, new_jval_s(completeMessage));
        pthread_mutex_unlock(client->chatRoom->lock);

        message = (char *) malloc(500 * sizeof(char));
    }

    /* When the client is finished entering messages (when they disconnect), remove them from the chat room's client list */
    dll_traverse(tmpClients, client->chatRoom->clients) {
        Thread_Info *tmpClient = (Thread_Info *) tmpClients->val.v;
        if (strcmp(tmpClient->chatName, client->chatName) == 0) {
            dll_delete_node(tmpClients);
            break;
        }
    }

    /* Close the fds */
    close(client->fd);
    fclose(client->fin);
    fclose(client->fout);

    /* Creating the leave message */
    char *leftMessage = (char *) malloc(500 * sizeof(char));
    sprintf(leftMessage, "%s has left\n", client->chatName);

    /* Adding the leave message to the dllist */
    pthread_mutex_lock(client->chatRoom->lock);
    dll_append(client->chatRoom->messages, new_jval_s(leftMessage));
    pthread_mutex_unlock(client->chatRoom->lock);

    /* Freeing our client memory */
    free(client->chatName);
    free(client);
    
    /* Detaching the thread */
    pthread_detach(pthread_self());
    return NULL;
}

/* Method that handles the threading for each chat room */
void *serverThread(void *room) {
    Room_Info *chatRoom = (Room_Info *) room;

    while (1) {
        if (!dll_empty(chatRoom->messages)) {
            Dllist tmpMessages = chatRoom->messages;

            /* Iterating through the messages */
            for (tmpMessages = tmpMessages->flink; tmpMessages != chatRoom->messages;) {
                
                /* If the message exists and isn't the sentinel node */
                if (tmpMessages != NULL && tmpMessages->flink != tmpMessages) {

                    /* Traverse the clients and print out the message for each of them */
                    Dllist tmpClient;
                    jrb_traverse(tmpClient, chatRoom->clients) {
                        if (tmpClient != NULL) {
                            Thread_Info *client = (Thread_Info *) tmpClient->val.v;
                            fprintf(client->fout, "%s", tmpMessages->val.s);
                            fflush(client->fout);
                        }
                    }

                    /* Storing the next node in the list */
                    Dllist flink = tmpMessages->flink;

                    /* Freeing the memory for the message and removing it from the dllist */
                    if (tmpMessages->val.s != NULL && tmpMessages != NULL) {
                        pthread_mutex_lock(chatRoom->lock);
                        free(tmpMessages->val.s);
                        dll_delete_node(tmpMessages);
                        pthread_mutex_unlock(chatRoom->lock);
                    }

                    /* If there aren't any more messages, we exit the loop */
                    if (chatRoom->messages == flink) {
                        break;
                    }

                    /* Going to the next message in the list */
                    tmpMessages = flink;
                }
                else {
                    break;
                }
            }
        }
    }
}