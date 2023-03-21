#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>

int flag = 0;
int handlerFinished = 0;
char *prompt = "hello";

void handle_sigint(int sig) {
    if (flag == 0) { // parent
        printf("\nYou typed Control-C!\n");
    } else if (flag == 1) { // child
        exit(0);
    }
}

int main() {
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status, processStatus;
    char *argv[10];
    pid_t pid;

    while (1) {
        signal(SIGINT, handle_sigint); // Register signal handler for SIGINT

        printf("%s: ", prompt);
        fgets(command, 1024, stdin);

        if (handlerFinished == 1) {
            continue;
        }

        command[strlen(command) - 1] = '\0';

        /* parse command line */
        i = 0;
        token = strtok(command, " ");
        while (token != NULL) {
            argv[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        argv[i] = NULL;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        if (strcmp(argv[0], "quit") == 0) {
            exit(0);
        }

        //changing prompt
        if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0) {
            prompt = argv[2];
            continue;
        }

        if (strcmp(argv[0], "cd") == 0 && argv[1] != NULL) {
            // Change the current working directory to example_dir
            if (chdir(argv[1]) != 0) {
                perror("chdir() error");
                exit(1);
            }

            char currentDirectory[1024];
            // Print the new current working directory
            if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL) {
                printf("New current working directory: \n%s\n", currentDirectory);
            } else {
                perror("getcwd() error");
                exit(1);
            }


        }

        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;

        if (strcmp(argv[0], "echo") == 0 && strcmp(argv[1], "$?") == 0) {
            printf("Previous command exited with status: %d\n", WEXITSTATUS(processStatus));
            continue;
        }

        if (!strcmp(argv[i - 2], ">")) {
            redirect = 1;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else if (!strcmp(argv[i - 2], "2>")) {
            redirect = 2;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else if (!strcmp(argv[i - 2], ">>")) {
            redirect = 3;
            argv[i - 2] = NULL;
            outfile = argv[i - 1];
        } else redirect = 0;

        /* for commands not part of the shell command language */
        pid = fork();
        if (pid == 0) {
            flag = 1;
            for (int i = 0; i < 100000; i++) {
                printf("%d\n", i);
            }
            /* redirection of IO ? */
            if (redirect == 1) {
                fd = creat(outfile, 0660);
                if (fd == -1) {
                    printf("Error in opening the file\n");
                    exit(1);
                }
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            } else if (redirect == 2) {
                fd = creat(outfile, 0660);
                if (fd == -1) {
                    printf("Error in opening the file\n");
                    exit(1);
                }
                close(STDERR_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            } else if (redirect == 3) {
                fd = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0660);
                if (fd == -1) {
                    printf("Error in opening the file\n");
                    exit(1);
                }
                close(STDOUT_FILENO);
                dup(fd);
                close(fd);
                /* stdout is now redirected */
            }
            execvp(argv[0], argv);
        } else {
            // parent process
            flag = 2;
            waitpid(pid, &processStatus, 0);
            flag = 0;
        }

        /* parent continues here */
        if (amper == 0)
            retid = wait(&status);
    }
}
