// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fields.h"

int main(int argc, char **argv) {
    char *prompt;
    int status;

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
        // printf("%s %s\n", prompt, is->text1);

        if (strcmp(is->text1, "exit\n") == 0 || strcmp(is->text1, "CNTL-D\n") == 0) {
            printf("Made it here");
            return 1;
        }

        if (is->NF > 1) {
            int i;
            printf("Printing fields\n");
            for (i = 0; i < is->NF; i++) {
                printf("[%s] ", is->fields[i]);
            }
            printf("\n");
            // printf("oi\n");
            if (fork() == 0) {
                // printf("yizzy wizzy yo\n");
                // printf("%s %s\n", is->fields[0], is->fields+1);
                execvp(is->fields[0], is->fields);
                perror("command1");
                exit(1);
            }
            else {
                // printf("yizzy yo\n");
                if (strcmp(is->fields[is->NF-1], "&") != 0) {
                    wait(&status);
                }
            }

        }
    }

    return 0;
}