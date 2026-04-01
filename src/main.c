#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cd-hist.h"

#define MAX_LINE 80 /* The max lenght cmd*/
#define MAX_HIST 10 // max number of commands to store in history
#define MAX_LEN 255 // max length of input command
#define MAX_PARAMS 12 // greatest number of parameters a command can have

int main(void) {

    char *args[(MAX_LINE / 2) + 1]; // cmd line args
    int should_run = 1; // flag to determine when to exit
    char *buf = malloc(MAX_LEN * sizeof(char));
    __pid_t pid, pid1;
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    memset(buf, 0, MAX_LEN*sizeof(char));
    char* command = malloc(MAX_LEN);
    if (command == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    char* params[MAX_PARAMS];
    int* is_wait = 0;
    
    while (should_run) {
        memset(params, 0, MAX_PARAMS * sizeof(char*));
        printf("$ cdsh> ");
        fflush(stdout);
        // scanf();
        fgets(buf, MAX_LEN*sizeof(char), stdin);
        // printf("do something with this: %s\n", *args);
        split(buf, command, params);
        if (strcmp(command, "exit") == 0) {
            should_run = 0;
            continue;
        }
        pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Fork failed\n");
            return 1;
        } else if (pid == 0) {
            // child process
            execvp(command, params);
            fprintf(stderr, "Command execution failed\n");
            return 1;
        } else if (is_wait) {
            // parent process
            // TODO need to deal with the ampersand now
            wait(NULL);
        
        } else {
            // parent process, this is running in the background so we don't wait for it to finish
            continue;
        }
        
    }
    free(buf);
    free(command);
    buf = NULL;
    return 0;
}

void split(char *input, char* command, char** params) {
    // TODO: split the input into command and parameters
    char* token = strtok(input, " ");
    // char** ptr = *&params;
    int count = 0;
    int last_idx = 0;
    char* temp;
    while (token != NULL) {
        token[strcspn(token, "\n")] = '\0'; // remove the newline character from the token if present
        if (count == 0) {
            strcpy(command, token);
        }

        count++;
        temp = strtok(NULL, " ");

        if (strcmp(token, "\n") == 0) {
            token = NULL;
            continue;
        }

        *params++ = token;
        token = temp;
        
    }
}
