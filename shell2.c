#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include "mylink.c"

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

/*
removes the newline and space character from the end and start of a char*
*/
void removeWhiteSpace(char *buf) {
    if (buf[strlen(buf) - 1] == ' ' || buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = '\0';
    if (buf[0] == ' ' || buf[0] == '\n') memmove(buf, buf + 1, strlen(buf));
}

/*
tokenizes char* buf using the delimiter c, and returns the array of strings in param
and the size of the array in pointer nr
*/
void tokenize_buffer(char **param, int *nr, char *buf, const char *c) {
    char *token;
    token = strtok(buf, c);
    int pc = -1;
    while (token) {
        param[++pc] = malloc(sizeof(token) + 1);
        strcpy(param[pc], token);
        removeWhiteSpace(param[pc]);
        token = strtok(NULL, c);
    }
    param[++pc] = NULL;
    *nr = pc;
}

/*
loads and executes a series of external commands that are piped together
*/
void executePiped(char **buf, int nr) {//can support up to 10 piped commands
    if (nr > 10) return;

    int fd[10][2], i, pc;
    char *argv[100];

    for (i = 0; i < nr; i++) {
        tokenize_buffer(argv, &pc, buf[i], " ");
        if (i != nr - 1) {
            if (pipe(fd[i]) < 0) {
                perror("pipe creating was not successfull\n");
                return;
            }
        }
        if (fork() == 0) {//child1
            if (i != nr - 1) {
                dup2(fd[i][1], 1);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if (i != 0) {
                dup2(fd[i - 1][0], 0);
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            execvp(argv[0], argv);
            perror("invalid input ");
            exit(1);//in case exec is not successfull, exit
        }
        //parent
        if (i != 0) {//second process
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
    }
}

int main() {
    char command[1024];
    char *token;
    char *outfile;
    int i, fd, amper, redirect, retid, status, processStatus;
    char *argv[10];
    int argc1;
    pid_t pid;
    int ifcounter=0;

    while (1) {
        signal(SIGINT, handle_sigint); // Register signal handler for SIGINT

        printf("%s: ", prompt);
        fgets(command, 1024, stdin);
        char buf[1024];
        strcpy(buf, command);

        

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
        int argc = i - 1;
        argc1 = i;

        /* Is command empty */
        if (argv[0] == NULL)
            continue;

        // pipe
        int nr = 0;
        if (strchr(buf, '|')) {
            char *buffer[100];
            tokenize_buffer(buffer, &nr, buf, "|");
            executePiped(buffer, nr);
            continue;
        }

        // exitting
        if (strcmp(argv[0], "quit") == 0) {
            exit(0);
        }

        // exit status
        if (strcmp(argv[0], "echo") == 0 && strcmp(argv[1], "$?") == 0) {
            printf("Previous command exited with status: %d\n", WEXITSTATUS(processStatus));
            continue;
        }


        // saving variable
        if (argc1 == 3 && strcmp(argv[1], "=") == 0 && ((argv[0][0]) == '$')) {
            insertFirst(argv[0], argv[2]);
            continue;
        }


        // read comand
        if (strcmp(argv[0], "read") == 0 && argc1>1){

            char reading[1024];
            fgets(reading,1024,stdin);
            char buf2[1024];
            strcpy(buf2, reading);
            reading[strlen(reading) - 1] = '\0';

            int k = 0;
            char *readargv[100];
            char *tokens = strtok(reading, " ");
            while (tokens != NULL) {
                readargv[k] = tokens;
                tokens = strtok(NULL, " ");
                k++;
            }
            readargv[k] = NULL;
            int argcRead = k;

            int j;
            for (j=0; j< argc && j<argcRead; j++){
                char v[1024]="$";
                strcat(v,argv[j+1]);
                insertFirst(v,readargv[j]);
            }
            if (j==argc && j <argcRead){
                j--;
                char v[1024]="$";
                strcat(v,argv[j+1]);
                char a[1024]="";
                for (int l=j; l<argcRead;l++){
                   strcat(a,readargv[l]);
                   if (l!=argcRead-1){
                   strcat(a, " ");
                   }
                }
                insertFirst(v,a);
            }

            if (j< argc && j== argcRead){
                for (int l=j; l<argc;l++){
                    char v[1024]="$";
                    strcat(v,argv[l+1]);
                    insertFirst(v,""); 
                }
            }
            continue;
        }



        // getting echo ready
        if (strcmp(argv[0], "echo") == 0 && argc1 > 1) {
            for (int j = 1; j < argc1; j++) {
                if (argv[j][0]=='$'){
                    struct node *x = find(argv[j]);
                    if (x != NULL) {
                        argv[j] = x->value;
                    }
                    else{
                        argv[j]="";
                    }
                }
            }
        }

        //changing prompt
        if (strcmp(argv[0], "prompt") == 0 && strcmp(argv[1], "=") == 0) {
            printf("changing the prompt\n");
            char* temp = (char*) malloc(sizeof (argv[2]));
            strcpy(temp, argv[2]);
            prompt = temp;
            continue;
        }

        // changing directory
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


        if (strcmp(argv[0],"if") == 0){
            
            char statment[4096] = "";
            char *semi=";";
            for (int j=0; j<argc1; j++){
                if (strcmp(argv[j],"if")==0){
                    ifcounter++;
                }
                if (strcmp(argv[j],"fi")==0){
                    ifcounter--;
                }
                struct node *x= find(argv[j]);
                if (x!= NULL){
                    argv[j]=x->value;
                }
                strcat(statment, argv[j]);
                strcat(statment," ");
            }
            strcat(statment, semi);
            printf("statment = %s\n",statment);
            printf("ifcounter= %d\n",ifcounter);
            
            
            while (ifcounter>0){
                printf(">");
                char reading[1024];
                fgets(reading,1024,stdin);
                char buf2[1024];
                strcpy(buf2, reading);
                reading[strlen(reading) - 1] = '\0';

                int k = 0;
                char *readargv[100];
                char *tokens = strtok(reading, " ");
                while (tokens != NULL) {
                    readargv[k] = tokens;
                    tokens = strtok(NULL, " ");
                    k++;
                }
                readargv[k] = NULL;
                int argcRead = k;

                for (int j=0; j<argcRead; j++){
                if (strcmp(readargv[j],"if")==0){
                    ifcounter++;
                }
                if (strcmp(readargv[j],"fi")==0){
                    ifcounter--;
                }
                struct node *x= find(readargv[j]);
                if (x!= NULL){
                    argv[j]=x->value;
                }
                strcat(statment, readargv[j]);
                strcat(statment," ");
            }
            strcat(statment, semi);
            strcat(statment, " ");
            // printf("statment = %s\n",statment);
            // printf("ifcounter= %d\n",ifcounter);
                
            }
            pid = fork();
            if (pid == 0) {
                flag = 1;
                system(statment);
            }
            else{
                flag = 2;
                waitpid(pid, &processStatus, 0);
                flag = 0;
            }

        }





        /* Does command line end with & */
        if (!strcmp(argv[i - 1], "&")) {
            amper = 1;
            argv[i - 1] = NULL;
        } else
            amper = 0;

        


        // redirects
        if (argc > 1) {
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
        }

        /* for commands not part of the shell command language */
        pid = fork();
        if (pid == 0) {
            flag = 1;
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
        } 
        else {
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
