#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "command.h"
#include "dynarray.h"
#include "signal.h"

#define STDOUT 1

// Variable to set whether we are accepting background commands or not
int accept_bg_commands = 1;

// Does all input and output redirection with a given command, sets p_fdin and p_fdout to the
// respective file descriptors that were changed.
int redirect_io(int* p_fdin, int* p_fdout, struct command* cmd) {
    int fdin = *p_fdin;
    int fdout = *p_fdout;
    // Redirects both input and output to /dev/null if run in the background
    if (cmd->background) {
        fdin = open("/dev/null", O_RDWR);
        fdout = fdin;
        dup2(fdin, 0);
        dup2(fdout, 1);
    }
    // Input Redirection
    if (cmd->input != NULL) {
        fdin = open(cmd->input, O_RDONLY);
        if (fdin == -1) {
            printf("Could not open %s for reading\n", cmd->input);
            return -1;
        }
        dup2(fdin, 0);
    }
    // Output Redirection
    if (cmd->output != NULL) {
        fdout = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRGRP | S_IROTH);
        if (fdout == -1) {
            printf("Could not open %s for writing\n", cmd->output);
            return -1;
        }
        dup2(fdout, 1);
    }

    return 0;
}

int child_main(struct command* cmd) {
    // Set the child to treat signals correctly depending on foreground status
    struct sigaction sigint;
    struct sigaction sigtstp;
    sigset_t mask;
    sigfillset(&mask);
    sigint.sa_flags = 0;
    sigtstp.sa_flags = 0;
    if (cmd->background)
        sigint.sa_handler = SIG_IGN;
    else
        sigint.sa_handler = SIG_DFL;
    sigtstp.sa_handler = SIG_IGN;
    sigint.sa_mask = mask;
    sigtstp.sa_mask = mask;
    sigaction(SIGINT, &sigint, NULL);
    sigaction(SIGTSTP, &sigtstp, NULL);

    int fdin = -1;
    int fdout = -1;
    // If input/ouput redirection fails, return with an exit status of 1
    if (redirect_io(&fdin, &fdout, cmd) == -1)
        return 1;

    // Execute the command, if an error occurs, print an error and return 1
    if (execvp(cmd->name, cmd->argv) == -1) {
        printf("%s: no such file or directory\n", cmd->name);
        return 1;
    }

    // Close any open io files
    if (fdin != -1)
        close(fdin);
    if (fdout != -1 && fdout != fdin)
        close(fdout);
    return 0;
}

// Prints out the status of the previously run program (not including built-ins)
void status(int exit_status) {
    if (WIFEXITED(exit_status)) {
        printf("Exit value %d\n", WEXITSTATUS(exit_status));
    }
    else if (WIFSIGNALED(exit_status)){
        printf("Terminated by signal %d\n", WTERMSIG(exit_status));
    }
}

// Changes the current working directory
void cd(struct command* command) {
    char* new_path;
    int err = 0;
    int result;
    if (command->argc == 1)
        new_path = getenv("HOME");
    else if (command->argc == 2)
        new_path = command->argv[1];
    else
        err = 1;

    if (err) {
        printf("Error: Too many arguments, cd only accepts 0 or 1 arguments.\n");
        return;
    }
    if (new_path == NULL) {
        printf("Error: HOME environment variable not set\n");
        return;
    }

    result = chdir(new_path);
    if (result != 0)
        printf("Error: Couldn't find the directory %s\n", new_path);
    
}

// Signal handler for SIGTSTP
void sig_handler_parent(int signal) {
    if (signal == SIGTSTP) {
        if (accept_bg_commands) {
            accept_bg_commands = 0;
            write(STDOUT, "\nEntering foreground-only mode (& is now ignored)\n", 51);
        }
        else {
            accept_bg_commands = 1;
            write(STDOUT, "\nExiting foreground-only mode\n", 31);
        }
    }
}

int main() {
    // 512 character buffer + null character
    char current_dir[513] = {0};
    getcwd(current_dir, 512);

    // 2048 character buffer + null character
    char input[2049] = {0};
    struct command* command = NULL;

    int bg_exit_status = 0;
    int exit_status = 0;
    int return_val = 0;

    struct array* bg_pids = arr_create();
    pid_t child_pid;

    // Signal handling for the parent process
    struct sigaction sigact;
    struct sigaction sigint;
    sigset_t mask;
    sigfillset(&mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = sig_handler_parent;
    sigact.sa_mask = mask;
    sigint.sa_flags = 0;
    sigint.sa_handler = SIG_IGN;
    sigint.sa_mask = mask;
    sigaction(SIGINT, &sigint, NULL);
    sigaction(SIGTSTP, &sigact, NULL);

    while (1) {
        // Set the first character of input to null so if an interrupt occurs during fgets, the parse_command will see it as a blank line
        input[0] = '\0';
        // print cwd
        getcwd(current_dir, 512);
        printf("%s: ", current_dir);
        fflush(stdout);     // Flush the output to work better with the grading script
        
        // Get user input and parse it into a command structure
        fgets(input, 2048, stdin);
        command = parse_command(input);
        if (!accept_bg_commands)
            command->background = 0;

        // Ignore comment lines
        if (command->name[0] == '#' || command->name[0] == '\0' ) {
        }
        // Three built in commands
        else if (strcmp("status", command->name) == 0) {
            status(exit_status);
            fflush(stdout);
        }
        else if (strcmp("cd", command->name) == 0) {
            cd(command);
        }
        else if (strcmp("exit", command->name) == 0) {
            free_command(command);
            free_array(bg_pids);
            return EXIT_SUCCESS;
        }
        // All other commands will be forked
        else {
            switch (child_pid = fork())
            {
            // Error occurred creating child process
            case -1:
                printf("Error: could not create child process\n");
                fflush(stdout);
                break;
            // The child will go here and call child_main
            case 0:
                return_val = child_main(command);
                free_command(command);
                free_array(bg_pids);
                return return_val;
            // The parent goes here and listens for the child process
            default:
                if (command->background) {
                    printf("Background pid is %d\n", child_pid);
                    fflush(stdout);
                    arr_insert(bg_pids, child_pid);
                    waitpid(child_pid, &exit_status, WNOHANG);
                }
                else
                    waitpid(child_pid, &exit_status, 0);
                break;
            }
        }

        // Check our list of background pids to see if any of them have finished
        for (int i = 0; i < bg_pids->size; i++) {
            if (waitpid(bg_pids->items[i], &bg_exit_status, WNOHANG) != 0) {
                printf("Background pid %d is done: ", bg_pids->items[i]);
                fflush(stdout);
                status(bg_exit_status);
                arr_remove(bg_pids, bg_pids->items[i]);
                i--;
            }
        }

        // Free the memory used by our command structure so we have no memory leaks
        free_command(command);
    }
}