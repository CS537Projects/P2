#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//mysh.c

//This literally is pretty much what we need to do, 
//we can use this annd basically go through most of the project
//          I
//          V
// https://brennan.io/2015/01/16/write-a-shell-in-c/

void turtle_mode(){
    int exit_found = 0;
    do{
        const char delim[2] = " ";
        char buf[512];
        char *token = malloc(sizeof(buf));
        char **array = malloc(sizeof(char *));

        write(1, "> ", 3);
        //get line
        fgets(buf, sizeof buf, stdin);
        strncpy(token, buf, strlen(buf)-1);
        //strncat(token, "\0", 2);
        token = strtok(token, delim);
        int i = 0;
        while(token != NULL) {
            array[i] = malloc(sizeof(token));
            strncpy(array[i], token, strlen(token));
            token = strtok(NULL, delim);
            i++;
        }
        if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else{
            
            pid_t pid = fork();
            int status;
            pid_t wait;
            if(pid == 0){
                //Child
                execvp(array[0], array);
                _exit(0);

            }else if(pid <0){
                //Error 
                exit(1);
            }else{
                do {
                    wait = waitpid(pid, &status, 0);
                } while (!WIFSIGNALED(status) && !WIFEXITED(status));
            }
        }
        for(int i = 0; i<sizeof(array); i++){
                free(array[i]);
        }
        free(array);
        //free(token);
        //_exit(0);
    }while(exit_found == 0);
    _exit(0);
}

int main(int argc, char **argv)
{
    if(argc == 1){
        //shell mode
        turtle_mode();
    }else if(argc == 2){
        //batch mode
    }else{
        //ERROR
    }


  // Perform any shutdown/cleanup.

  return 0;
}
