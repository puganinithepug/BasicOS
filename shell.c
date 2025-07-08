#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>

#include "limits.h"
#include "shell.h"
#include "interpreter.h"
#include "varstore.h"
#include "codestore.h"
#include "scheduler.h"

#define CMD_DELIM ";"
#define PROMPT '$'

// Start of everything
int main(int argc, char *argv[]) {
    printf("Frame Store Size = %d; Variable Store Size = %d\n", MEMORY_MAX_LINES, MEM_SIZE);
    // printf("Shell version 1.3 created September 2024\n\n");
    //help();

    // init shell memory
    mem_init();

    // init code store
    init_code_store();

    return run_shell(stdin);
}

// Run the shell on a specified stream
int run_shell(FILE *input_stream) {
    char userInput[MAX_USER_INPUT];		// user's input stored here

    // Initialize user input
    memset(userInput, 0, sizeof(userInput));

    // Is the shell in interactive mode?
    int shellIsInteractive = isatty(fileno(input_stream));

    // Main REPL
    char *ret_val;      // fgets return value
    while(1) {
        // Print the prompt
        if (shellIsInteractive) { printf("%c ", PROMPT); }

        // Get input
        ret_val = fgets(userInput, MAX_USER_INPUT-1, input_stream);

        if (ret_val == NULL) {
            // Reached EOF
            printf("\n");
            break;  // exit
        } else {
            execute_line(userInput);
        }
        memset(userInput, 0, sizeof(userInput));
    }

    codestore_terminate();
    scheduler_free();

    return 0;
}

// Run a line of code
void execute_line(char *line) {
    char *cmd;          // command to execute
    int errorCode;      // command error code

    // Split one-liners
    cmd = strtok(strdup(line), CMD_DELIM);
    while (cmd != NULL) {
        errorCode = parseInput(cmd);        // run the command
        if (errorCode == -1) exit(99);	// ignore all other errors
        cmd = strtok(NULL, CMD_DELIM);
    }
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

int parseInput(char inp[]) {
    char tmp[200], *words[100];                            
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    for (ix = 0; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces
    while (inp[ix] != '\n' && inp[ix] != '\0' && ix < 1000) {
        // extract a word
        for (wordlen = 0; !wordEnding(inp[ix]) && ix < 1000; ix++, wordlen++) {
            tmp[wordlen] = inp[ix];                        
        }
        tmp[wordlen] = '\0';
        words[w] = strdup(tmp);
        w++;
        if (inp[ix] == '\0') break;
        ix++; 
    }
    errorCode = interpreter(words, w);
    return errorCode;
}

