// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"

int main(int argc, char **argv) {
    char *prompt;
    int status;
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
        // printf("Got line: %s\n", is->text1);

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