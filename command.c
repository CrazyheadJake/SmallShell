#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "command.h"

// Expands the $$ variable into the pid of smallsh
char* expand_variable(char* input) {
    char* command = malloc(sizeof(char) * (strlen(input)+1));
    strcpy(command, input);
    char* temp;
    pid_t pid = getpid();
    char* pid_str = malloc(sizeof(char) * 7);
    sprintf(pid_str, "%d", pid);

    for (int i = 0; i < strlen(command); i++) {
        if (command[i] == '$' && command[i+1] == '$') {
            temp = calloc(strlen(command) + strlen(pid_str)+1, sizeof(char));
            if (i != 0)
                strncpy(temp, command, i);
            strcat(temp, pid_str);
            strncat(temp, &(command[i+2]), strlen(command)-(i+2));
            free(command);
            command = temp;
        }
    }
    free(pid_str);

    return command;
}

void add_arg(struct command* cmd, char* arg) {
    cmd->argv[cmd->argc] = malloc(sizeof(char) * (strlen(arg) + 1));
    strcpy(cmd->argv[cmd->argc], arg);
    cmd->argc++;
}

struct command* parse_command(char* command) {
    // For use with strtok_r
    char* saveptr;

    struct command* cmd = malloc(sizeof(struct command));


    // Expand all $$ to pid
    command = expand_variable(command);

    // The first token is the title
    char* token = strtok_r(command, " \n", &saveptr);
    if (token) {
        cmd->name = calloc(strlen(token) + 1, sizeof(char));
        strcpy(cmd->name, token);
    }
    else {
        cmd->name = calloc(1, sizeof(char));
        cmd->name[0] = '\0';
    }

    cmd->argv = calloc(513, sizeof(char*));
    // Set the first argument of argv to be the command name as this is the way we need to pass it into execvp
    cmd->argv[0] = malloc(sizeof(char) * (strlen(cmd->name) + 1));
    strcpy(cmd->argv[0], cmd->name);

    cmd->background = 0;
    cmd->input = NULL;
    cmd->output = NULL;
    int bg = 0;
    cmd->argc = 1;

    // Iterate over all arguments and parse them
    while (token = strtok_r(NULL, " \n", &saveptr)) {
        if (strcmp(token, ">") == 0) {
            if (bg) {
                bg = 0;
                add_arg(cmd, "&");
            }
            token = strtok_r(NULL, " \n", &saveptr);
            cmd->output = malloc(sizeof(char) * (strlen(token) + 1));
            strcpy(cmd->output, token);
        }
        else if (strcmp(token, "<") == 0) {
            if (bg) {
                bg = 0;
                add_arg(cmd, "&");
            }
            token = strtok_r(NULL, " \n", &saveptr);
            cmd->input = malloc(sizeof(char) * (strlen(token) + 1));
            strcpy(cmd->input, token);
        }
        else if (strcmp(token, "&") == 0) {
            bg = 1;
        } 
        else {
            if (bg) {
                add_arg(cmd, "&");
                bg = 0;
            }
            add_arg(cmd, token);
        }
    }
    if (bg) {
        cmd->background = 1;
    }
    free(command);
    return cmd;
}

// Free all memory used by command
void free_command(struct command* cmd) {
    free(cmd->name);
    if (cmd->input)
        free(cmd->input);
    if (cmd->output)
        free(cmd->output);
    for (int i = 0; i < cmd->argc; i++) {
        free(cmd->argv[i]);
    }
    free(cmd->argv);
    free(cmd);
}