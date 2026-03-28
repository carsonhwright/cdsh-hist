#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cd-hist.h"

#define MAX_LINE 80 /* The max lenght cmd*/
#define MAX_HIST 10 // max number of commands to store in history
#define MAX_LEN 255 // max length of input command
#define MAX_PARAMS 5 // greatest number of parameters a command can have

int main(void) {

    char *args[(MAX_LINE / 2) + 1]; // cmd line args
    int should_run = 1; // flag to determine when to exit
    char *buf = malloc(MAX_LEN * sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    memset(buf, 0, MAX_LEN*sizeof(char));
    char* command;
    char* params[MAX_PARAMS];
    memset(params, 0, MAX_PARAMS * sizeof(char*));
    while (should_run) {
        printf("$ cdsh> ");
        fflush(stdout);
        // scanf();
        fgets(buf, MAX_LEN*sizeof(char), stdin);
        // printf("do something with this: %s\n", *args);
        split(buf, command, params);
        
        // TODO: command|params need tp be parsed by split(), ma
        execvp(command, params);
        if (strcmp(command, "exit") == 0) {
            should_run = 0;
        }
    }
    free(buf);
    buf = NULL;
    return 0;
}

void split(char *input, char* command, char** params) {
    // TODO: split the input into command and parameters
    char* token = strtok(input, " ");
    // char** ptr = *&params;
    int count = 0;
    while (token != NULL) {
        if (count == 0) {
            strcpy(command, token);
        }
        count++;
        *params++ = token;
        token = strtok(NULL, " ");
    }

}