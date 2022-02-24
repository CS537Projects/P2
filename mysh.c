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
        char *token;
        char **array;

        char ***aliasArray;

        array = malloc(sizeof(char *));
        aliasArray = malloc(sizeof(char **));

        printf("> ");
        //get line
        fgets(buf, sizeof buf, stdin);
        strncpy(token, buf, strlen(buf)-1);
        strncat(token, "\0", 2);

        token = strtok(token, delim);
        int i = 0;
        while(token != NULL) {
            array[i] = malloc(sizeof(token));
            strncpy(array[i], token, strlen(token));
            token = strtok(NULL, delim);
            i++;
        }

        //check array[0] for alias
        //go through array, check for >>
            //The exact format of redirection is: a command (along with its arguments, 
            //if present), followed by any number of white spaces (including none), 
            //the redirection symbol >,  again any number of white space (including none), 
            //followed by a filename.
        execvp(array[0], array);


        //fork bs with array bs 
        
        //fork()bs
        //do command

        exit_found = 1;

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
