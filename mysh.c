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
    //char *temp;
    do{
        const char delim[2] = " ";
        char buf[512];
        char *token = malloc(sizeof(buf));
        char **array = malloc(sizeof(char *));

        write(1, "> ", 3);
        fgets(buf, sizeof buf, stdin);
        buf[strcspn(buf, "\n")] = 0;
        token = strtok(buf, delim);

        int j = 0;
        while(token != NULL) {
            array[j] = malloc(sizeof(token));
            strncpy(array[j], token, strlen(token));
            token = strtok(NULL, delim);
            j++;
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
                array[i] = NULL;
        }
        free(array);
    }while(exit_found == 0);
    _exit(0);
}


void bachelorette_mode(char *file){
    
    
    
    FILE *fp = fopen(file, "r");
        if (fp == NULL) {
        printf("my-look: cannot open file\n");
        _exit(1);
    }
    int exit = 0;
    char buf[512];
    while ((fgets(buf, sizeof(buf), fp) != NULL) && (exit == 0)) {
        const char delim[2] = " ";
        char *token = malloc(sizeof(buf));
        char **array = malloc(sizeof(char *));
        //fgets(buf, sizeof(buf), fp);
        buf[strcspn(buf, "\n")] = 0;
        token = strtok(buf, delim);

        int j = 0;
        while(token != NULL) {
            array[j] = malloc(sizeof(token));
            strncpy(array[j], token, strlen(token));
            token = strtok(NULL, delim);
            j++;
        }
        if(strncmp(array[0], "exit",512) == 0){
            exit = 1;
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
                _exit(1);
            }else{
                do {
                    wait = waitpid(pid, &status, 0);
                } while (!WIFSIGNALED(status) && !WIFEXITED(status));
            }
        }

        for(int i = 0; i<sizeof(array); i++){
                array[i] = NULL;
        }
        free(array);
    }
    fclose(fp);

}


int main(int argc, char **argv)
{
    if(argc == 1){
        //shell mode
        turtle_mode();
    }else if(argc == 2){
        //batch mode
        bachelorette_mode(argv[1]);
    }else{
        //ERROR
    }


  // Perform any shutdown/cleanup.

  return 0;
}
