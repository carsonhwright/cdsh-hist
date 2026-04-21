#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <regex.h>
#include "cd-hist.h"

#define MAX_LINE 80 /* The max lenght cmd*/
#define MAX_LEN 255 // max length of input command
/*  The max size of the history buffer, this is not the same as the number of commands that can be 
    stored in history, but it is a limit on the total size of the history buffer. */
#define HISTORY_BUF_SIZE MAX_LEN * (MAX_HIST + 1)
#define MAX_PARAMS 12 // greatest number of parameters a command can have

/*
    main loop for shell, takes simple, un-piped commands and executes in the background. If 
    ampersand is at the end of the command, the parent will wait. Kind of an inverse of normal 
    behavior
    
    
    TODO history functionality is not implemented yet, but it will be added in the future.

*/
int main(void) {

    int should_run = 1; // flag to determine when to exit main loop
    char *buf = malloc(MAX_LEN * sizeof(char));
    regex_t task_repeat_regex;
    if (regcomp(&task_repeat_regex, "!(!|1?[0-9])", REG_EXTENDED)) {
        fprintf(stderr, "REGEX compilation error! Exiting.\n");
        return 1;
    }

    // buffer for history of previous commands
    struct history hist_buf;
    memset(hist_buf.commands, 0, MAX_HIST*sizeof(struct command));
    hist_buf.top = 0;
    hist_buf.commands[0].counter = 0;

    int is_wait = 0; // flag to determine if the command should be run in the background or not
    __pid_t pid;

    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    char* command = malloc(MAX_LEN);
    if (command == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    char* params[MAX_PARAMS];
    
    
    while (should_run) {
        // setup params for the next command
        memset(params, 0, MAX_PARAMS * sizeof(char*));
        memset(buf, 0, MAX_LEN*sizeof(char));

        // print the "shell" and get input
        printf("$ cdsh> ");
        fflush(stdout);
        fgets(buf, MAX_LEN*sizeof(char), stdin);

        if (rollback(buf, &hist_buf, &task_repeat_regex)) {
            // historical rollback command was incorrect
            continue;
        }
        push_history(&hist_buf, buf);
        split(buf, command, params, &is_wait);
        if (strcmp(command, "exit") == 0) {
            should_run = 0;
            continue;
        } else if (strcmp(command, "history") == 0) {
            for(int i=0; i <= (sizeof(hist_buf.commands) / sizeof(struct command)) - 1; i++) {
                // as long as the buffer entry isn't empty, print it
                if (hist_buf.commands[i].command != 0){
                    printf("%d %s", hist_buf.commands[i].counter, hist_buf.commands[i].command);
                }
            }
            // nothing needs to be executed by the system
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
            int status;

            waitpid(pid, &status, 0);
            
            if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);
                printf("child exited %d\n", code);
            } else if (WIFSIGNALED(status)) {
                printf("child killed by signal %d\n", WTERMSIG(status));
            }
            
            is_wait = 0;
            continue;
        
        } else {
            // parent process, this is running in the background so we don't wait for it to finish
            continue;
        }
        
    }
    free(buf);
    free(command);
    regfree(&task_repeat_regex);
    buf = NULL;
    return 0;
}

/*
    Formats the input into a set of parameters to be consumed by execvp. The first parameter is 
    the command, and the rest are the arguments. If the last parameter is an ampersand, the 
    `is_wait` flag will be set to 1
    */
void split(char *input, char* command, char** params, int* is_wait) {
    
    char* token = strtok(input, " ");
    int count = 0;
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
        if (strcmp(token, "&") == 0) {
            *is_wait = 1;
            token = NULL;
        }

        *params++ = token;
        token = temp;
    }
}

/*
    if !!, or !N was sent to shell, retreive the most recent or Nth most recent command 
    respectively and overwrite the buffer with it so that it can be executed
*/
int rollback(char* buffer, struct history* history, regex_t* regexp) {
    int regex_err;
    int nth_cmd = 0;
    regex_err = regexec(regexp, buffer, 0, NULL, 0);
    // printf(buffer);
    if (!regex_err) {
        
        if (buffer[1] == '!') {
            // get the latest response
            memset(buffer, 0, MAX_LEN*sizeof(char));
            strcpy(buffer, history->commands[history->top - 1].command);
            return 0;
        } else {
            // TODO this won't work for values greater than 9
            nth_cmd = buffer[1] - '0';
            if (nth_cmd < 0 || nth_cmd > 9) {
                return 1;
            } else if (history->top - (nth_cmd + 1) < 0) {
                // user gave an argument greater than current buffer position, and buffer not 
                // full, setting to first cmd
                nth_cmd = history->top;
            }
            memset(buffer, 0, MAX_LEN*sizeof(char));
            strcpy(buffer, history->commands[history->top - (nth_cmd + 1)].command);
            return 0;
        }
    }
    return 0;
}

/*
    history.commands are a readonly circular buffer, this updates said buffer and updates some 
    niceness for printing history if ever it is accessed
*/
void push_history(struct history* history, char* command) {

    if (history->top == MAX_HIST) {

        for (int i = 0; i < MAX_HIST - 1; i++) {
            history->commands[i] = history->commands[i+1];
        }
        history->top--;
        
    }

    if (!history->top) {
        history->commands[history->top].counter = 1;
    } else {
        history->commands[history->top].counter = 
            history->commands[(history->top - 1)].counter + 1;
    }
    history->commands[history->top].command = malloc(strlen(command) + 3); //still don't remember why + 3
    strcpy(history->commands[history->top++].command, command);
}
