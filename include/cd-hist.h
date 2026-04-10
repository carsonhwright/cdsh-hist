#define MAX_HIST 10 // max number of commands to store in history

struct history {
        char* commands[10];
        int top; // index of the most recent command in the history buffer
};

int main();
void split(char* unf_string, char* command, char** params, int* is_wait);
void push_history(struct history* history, char* command);