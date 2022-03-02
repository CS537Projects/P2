#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
//mysh.c

//This literally is pretty much what we need to do, 
//we can use this annd basically go through most of the project
//          I
//          V
// https://brennan.io/2015/01/16/write-a-shell-in-c/

void Kcopy(char* from, char* to){
    int j = 0;
    for (j = 0; from[j] != '\0'; ++j) {
        to[j] = from[j];
    }
    to[j] = '\0';
}


void turtle_mode(){
    int exit_found = 0;
    do{
        const char delim[2] = " ";
        char buf[512];
        char **array = malloc(sizeof(char *) * sizeof(char**));
        write(1, "> ", 3);
        fgets(buf, sizeof buf, stdin);
        buf[strcspn(buf, "\n")] = 0;
        char* token = strtok(buf, delim);
        int length = 0;
        while(token != NULL) {
            array[length] = malloc(sizeof(token) + 1);
            Kcopy(token, array[length]);
            token = strtok(NULL, delim);
            length++;
        }
        if(strlen(buf) == 0){
        }else if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            write(1, "alias detected\n", 16);
        }else if(strncmp(array[0], "unalias",512) == 0){
            write(1, "unalias detected\n", 18);
        }else{
                int arrayIndex = -1; //index in array where < is found
                int position = -1; //index inside of string where < is found
                int counter = 0;
            const char comparison[] = ">";
            for(int i = 0; i < length; i++){
                int temp = strcspn(array[i],comparison);
                if(temp < strlen(array[i])){
                    counter++;
                    arrayIndex = i;
                    position = temp;
                }
            }
            char file_name[512];
            int det = 0;
            if(counter != 0){
                det = 1;
                // more than one > 
                // there is nothing after > 
                // there is something after > and another array position
                // > not in last two positions
                if (counter > 1 
                    || (position == strlen(array[arrayIndex]) - 1 && arrayIndex == length - 1) 
                    || (position != strlen(array[arrayIndex]) - 1 && arrayIndex != length - 1)
                    || (arrayIndex < length - 2)){
                    write(1, "Redirection misformatted.\n", 27);
                }
                char **tempArray = malloc(sizeof(char *) * length * sizeof(char**));
                char *temp = malloc(sizeof(array[arrayIndex]));
                Kcopy(array[arrayIndex], temp);
                //There is something after > in the same string
                if(position != strlen(array[arrayIndex]) - 1){
                     //get b for file name
                     if(position != 0){
                         token = strtok(temp, comparison);
                         token = strtok(NULL, comparison);
                         Kcopy(token, file_name);
                         Kcopy(temp, array[arrayIndex]);
                         //A>B  works
                     }else{
                         token = strtok(temp, comparison);
                         Kcopy(token, file_name);
                         free(array[arrayIndex]);
                         //A >B work
                        length--;
                     }
                }else{
                    Kcopy(array[arrayIndex + 1], file_name);
                    if(position != 0){
                        free(array[arrayIndex + 1]);
                        int j = 0;
                         for (j = 0; temp[j] != '>'; ++j) {
                            array[arrayIndex][j] = temp[j];
                         }
                         array[arrayIndex][j] = '\0';
                        length--;
                        //A> B  does not work
                    }else{
                        free(array[arrayIndex]);
                        free(array[arrayIndex + 1]);
                        length = length - 2;
                        //A > B work
                    }
                }
                //for each string
                for(int i = 0; i < length; i++){
                    tempArray[i] = malloc(sizeof(strlen(array[i])));
                    Kcopy(array[i], tempArray[i]);
                }
                for(int i = 0; i < length; i++){
                    free(array[i]); //have to free more based on choices
                }
                free(array);
                array = tempArray;
                for(int i = 0; i < length; i++){
                    printf("arr:_%s_\n", array[i]); //have to free more based on choices
                }
                printf("file:_%s_\n", file_name);
                free(temp); 
            }
            pid_t pid = fork();
            int status;
            if(pid == 0){
                //Child
                if(det == 1){
                    int saved_stdout = dup(1);
                    close(STDOUT_FILENO);
                    int desc = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if(dup2(desc, STDIN_FILENO) < 0) {
                        //Is this the correct error?
                        printf("Cannot write to file %s.\n", file_name);
                    }
                    execvp(array[0], array);
                    close(desc);
                    dup2(saved_stdout, 1);
                    close(saved_stdout);
                }else{
                    execvp(array[0], array);
                }
                _exit(0);
            }else if(pid <0){
                //Error 
                _exit(1);
            }else{
                do {
                    waitpid(pid, &status, 0);
                } while (!WIFSIGNALED(status) && !WIFEXITED(status));
            }
        }
        for(int i = 0; i<length; i++){
                free(array[i]);
        }
        free(array);
        //free(token); 
    }while(exit_found == 0);
    _exit(0);
}

void bachelorette_mode(char *file){   
    FILE *fp = fopen(file, "r");
        if (fp == NULL) {
            write(STDERR_FILENO, "Error: Cannot open file.\n", 26);
            _exit(1);
        }
    int exit_found = 0;
    char buf[512];
    while ((fgets(buf, sizeof(buf), fp) != NULL) && (exit_found == 0)){
        const char delim[2] = " ";
        char **array = malloc(sizeof(char *) * sizeof(char**));
        buf[strcspn(buf, "\n")] = 0;
        char *token = strtok(buf, delim);
        int length = 0;
        while(token != NULL) {
            array[length] = malloc(sizeof(token) + 1);
            Kcopy(token, array[length]);
            token = strtok(NULL, delim);
            length++;
        }
        if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            write(1, "alias detected\n", 16);
        }else if(strncmp(array[0], "unalias",512) == 0){
            write(1, "unalias detected\n", 18);
        }else{
                int arrayIndex = -1; //index in array where < is found
                int position = -1; //index inside of string where < is found
                int counter = 0;
            const char comparison[] = ">";
            for(int i = 0; i < length; i++){
                int temp = strcspn(array[i],comparison);
                if(temp < strlen(array[i])){
                    counter++;
                    arrayIndex = i;
                    position = temp;
                }
            }
            char file_name[512];
            int det = 0;
            if(counter != 0){
                det = 1;
                // more than one > 
                // there is nothing after > 
                // there is something after > and another array position
                // > not in last two positions
                if (counter > 1 
                    || (position == strlen(array[arrayIndex]) - 1 && arrayIndex == length - 1) 
                    || (position != strlen(array[arrayIndex]) - 1 && arrayIndex != length - 1)
                    || (arrayIndex < length - 2)){
                    write(1, "Redirection misformatted.\n", 27);
                }
                char **tempArray = malloc(sizeof(char *) * length * sizeof(char**));
                char *temp = malloc(sizeof(array[arrayIndex]));
                Kcopy(array[arrayIndex], temp);
                //There is something after > in the same string
                if(position != strlen(array[arrayIndex]) - 1){
                     if(position != 0){
                         token = strtok(temp, comparison);
                         token = strtok(NULL, comparison);
                         Kcopy(token, file_name);
                         Kcopy(temp, array[arrayIndex]);
                         //A>B
                     }else{
                         token = strtok(temp, comparison);
                         Kcopy(token, file_name);
                         free(array[arrayIndex]);
                         length--;
                         //A >B
                     }
                }else{
                    Kcopy(array[arrayIndex + 1], file_name);
                    if(position != 0){
                        free(array[arrayIndex + 1]);
                        int j = 0;
                         for (j = 0; temp[j] != '>'; ++j) {
                            array[arrayIndex][j] = temp[j];
                         }
                         array[arrayIndex][j] = '\0';
                        length--;
                        //A> B
                    }else{
                        free(array[arrayIndex]);
                        free(array[arrayIndex + 1]);
                        length = length - 2;
                        //A > B
                    }
                }
                //for each string
                for(int i = 0; i < length; i++){
                    tempArray[i] = malloc(sizeof(strlen(array[i])));
                    Kcopy(array[i], tempArray[i]);
                }
                for(int i = 0; i < length; i++){
                    free(array[i]);
                }
                free(array);
                array = tempArray;
                free(temp);
                
            }
            //open
            pid_t pid = fork();
            int status;
            if(pid == 0){
                //Child
                if(det == 1){
                    int saved_stdout = dup(1);
                    close(STDOUT_FILENO);
                    int desc = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                    if(dup2(desc, STDIN_FILENO) < 0) {
                        //Is this the correct error?
                        printf("Cannot write to file %s.\n", file_name);
                    }
                    execvp(array[0], array);
                    close(desc);
                    dup2(saved_stdout, 1);
                    close(saved_stdout);
                }else{
                    execvp(array[0], array);
                }
                _exit(0);

            }else if(pid <0){
                //Error 
                _exit(1);
            }else{
                do {
                    waitpid(pid, &status, 0);
                } while (!WIFSIGNALED(status) && !WIFEXITED(status));
                //close
            }
        }

        for(int i = 0; i < length; i++){
                free(array[i]);
        }
        free(array);
        free(token);
    } 
    fclose(fp);
    _exit(0);

} 
int main(int argc, char **argv) {
    if(argc == 1){
        //shell mode
        turtle_mode();
    }else if(argc == 2){
        //batch mode
        bachelorette_mode(argv[1]);
    }else{
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", 26);
        _exit(1);
    }
  return 0;
}
