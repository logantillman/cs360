// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fields.h"

int main(int argc, char **argv) {
    char *prompt;
    int status, i;
    pid_t childPid, wpid;

    if (argc > 1) {
        prompt = argv[1];
    }
    else {
        prompt = (char *) malloc(6 * sizeof(char));
        strcpy(prompt, "jsh: ");
    }

    if (strcmp(prompt, "-") == 0) {
        strcpy(prompt, "");
    }

    IS is = new_inputstruct(NULL);

    while (get_line(is) >= 0) {

        if (strcmp(is->text1, "exit\n") == 0 || strcmp(is->text1, "CNTL-D\n") == 0) {
            printf("Made it here");
            return 1;
        }

        if (is->NF > 0) {
            int i;
            int shouldWait = 1;
            
            /* Checking for an ampersand to see if we should wait on child task to finish */
            if (strcmp(is->fields[is->NF-1], "&") == 0) {
                shouldWait = 0;
            }
            
            /* Storing if the string contains an ampersand */
            int containsAmp = !shouldWait;

            if ((childPid = fork()) == 0) {

                /* If there's an ampersand, we just change it to NULL */
                if (containsAmp) {
                    is->fields[is->NF-1] = NULL;
                }

                /* If there isn't an ampersand, we change the next free index to NULL */
                else {
                    is->fields[is->NF] = NULL;
                }

                int fd1, fd2, fd3;

                /* Checking each field in the line */
                for (i = 0; i < is->NF; i++) {
                    if (is->fields[i] == NULL) {
                        continue;
                    }

                    /* If it's "<", open the file as stdin */
                    if (strcmp(is->fields[i], "<") == 0) {
                        fd1 = open(is->fields[i+1], O_RDONLY);
                        if (fd1 < 0) {
                            perror("fd1 open:");
                            exit(1);
                        }

                        if (dup2(fd1, 0) != 0) {
                            perror("fd1 dup2:");
                            exit(1);
                        }

                        if (close(fd1) < 0) {
                            perror("c1");
                            exit(1);
                        }
                        is->fields[i] = NULL;
                    }

                    /* If it's ">", open the file as stdout. Allow truncation */
                    else if (strcmp(is->fields[i], ">") == 0) {
                        fd2 = open(is->fields[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                        if (fd2 < 0) {
                            perror("fd2 open:");
                            exit(2);
                        }

                        if (dup2(fd2, 1) != 1) {
                            perror("fd2 dup2:");
                            exit(1);
                        }

                        if (close(fd2) < 0) {
                            perror("c2");
                            exit(1);
                        }
                        is->fields[i] = NULL;
                    }

                    /* If it's ">>", open as stdout. No truncation, only append */
                    else if (strcmp(is->fields[i], ">>") == 0) {
                        fd3 = open(is->fields[i+1], O_WRONLY | O_APPEND | O_CREAT, 0644);
                        if (fd3 < 0) {
                            perror("fd3 open:");
                            exit(2);
                        }

                        if (dup2(fd3, 1) != 1) {
                            perror("fd3 dup2:");
                            exit(1);
                        }

                        if (close(fd3) < 0) {
                            perror("c3");
                            exit(1);
                        }
                        is->fields[i] = NULL;
                    }
                }

                /* Calling exec on the array */
                execvp(is->fields[0], is->fields);
                perror(is->fields[0]);
                exit(1);
            }

            /* For the parent process */
            else {

                /* Waiting if there was no ampersand specified */
                if (shouldWait) {

                    /* Waiting until the child process finishes (wpid variable for error checking) */
                    while ((wpid = wait(&status)) != childPid);
                }
            }

        }
    }

    return 0;
}