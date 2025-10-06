#ifndef __COMMAND_H
#define __COMMAND_H

// Structure to hold command information
struct command {
    char* name;
    int argc;
    char** argv;
    char* input;
    char* output;
    int background;
};

struct command* parse_command(char* command);
void free_command(struct command* cmd);

#endif
