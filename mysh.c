#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
//mysh.c

//This literally is pretty much what we need to do, 
//we can use this annd basically go through most of the project
//          I
//          V
// https://brennan.io/2015/01/16/write-a-shell-in-c/

int isEmpty(char* phrase){
    //printf("in herer\n");
    int length = strlen(phrase);
    //printf("in herer2\n");
    for(int i = 0; i< length; i++){
        if(!isspace(phrase[i])){
            return 0;
        }
    }
    //printf("in herer3\n");
    return 1;
}

void Kcopy(char* from, char* to){
    int j = 0;
    for (j = 0; from[j] != '\0'; ++j) {
        to[j] = from[j];
    }
    to[j] = '\0';
}

char* jump(char* command){
    while(isspace(*command)) {
        command++;
    }
    return command;
}

struct node {
    struct node* child;
    char name[512];
    char* arg[512];
};

struct node* head = NULL;

int search_alias(char *name, struct node *curr, char ***returnArray){
    if(head == NULL){
        return 1;
    }

    if(strcasecmp(curr->name, name) == 0){
        return 0;
        *returnArray = curr->arg;
    }
    
    if(curr-> child == NULL){
        return 1;
    }

    return 1;
}

void store_alias(char *name, char** val){
    char** emptyArray;
    
    if(search_alias(name,head, &emptyArray)==0){
        return;
    }

    char danger1[] = "alias";
    char danger2[] = "unalias";
    char danger3[] = "exit";


    if((strcasecmp(danger1, name)==0) || (strcasecmp(danger2, name)==0) || (strcasecmp(danger3, name)==0)){
        return;
    }

    struct node *addNode = (struct node*) malloc(sizeof(struct node));
    strcpy(addNode->name, name);
    for(int i = 2; i< sizeof(val); i++){
        //char *cpy = addNode->arg[i];
        Kcopy(addNode->arg[i], val[i]);
    }
    if(head!= NULL){
        addNode->child = head->child;
    }
    head = addNode;
}

int unalias(char *name, struct node *curr){
    if(strcasecmp(head->name, name)){
        struct node *removeNode = head;
        head = head->child;
        free(removeNode);
        return 1;
    }

    if(curr-> child == NULL){
        return 0;
    }

    if(strcasecmp(curr->child->name, name) == 0){
        struct node *removeNode = curr->child;
        curr->child = curr->child->child;
        free(removeNode);
        return 1;
    }

    return 0;
}


void execute(char** array){
    if(!isEmpty(array[0])){
        char** aliasArray;
        if(search_alias(array[0], head, &aliasArray)== 0){
                //call exec with alias
                execv((char*)aliasArray[0], aliasArray);
            }else{                            
                execv(array[0], array);
                fprintf(stderr, "%s: Command not found.\n", array[0]);  
            }
    }
}


void turtle_mode(){
    int exit_found = 0;
    do{
        const char delim[2] = " ";
        char buf[512];
        char **array = malloc(sizeof(char *) * sizeof(char**));
        write(1, "mysh> ", 6);
        if(fgets(buf, sizeof buf, stdin) == NULL){
            exit_found = 1;
            continue;
        }
        
        buf[strcspn(buf, "\n")] = 0;
        char* token = strtok(buf, delim);
        int length = 0;
        while(token != NULL) {
            if (!isEmpty(token)) {
                array[length] = malloc(sizeof(token) + 1);
                Kcopy(token, array[length]);
                length++;
            }

            token = strtok(NULL, delim);
        }
        array[length] = NULL;
        if(strlen(buf) == 0){
        }else if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            write(1, "alias detected\n", 16);
            store_alias(array[1], array);

        }else if(strncmp(array[0], "unalias",512) == 0){
            write(1, "unalias detected\n", 18);
            unalias(array[1], head);
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
                    write(2, "Redirection misformatted.\n", 26);
                    continue;
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
                free(temp); 
            }


            pid_t pid = fork();
            int status;
           if(pid == 0){
                //Child
                if(det == 1){
                    

                    if(isEmpty(file_name)){
                        fprintf(stderr, "Redirection misformatted.\n");
                        exit(1);
                        //continue;
                    }else{
                        //int saved_stdout = dup(1);
                        //close(STDOUT_FILENO);
                        int desc = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                        if(dup2(desc, STDOUT_FILENO) < 0) {
                        //Is this the correct error?
                            printf("Cannot write to file %s.\n", file_name);
                        }
                        close(desc);
                        execute(array);
                       
                        //dup2(saved_stdout, 1);
                        //close(saved_stdout);
                        _exit(1);
                    }
                    
                }else{
                    execute(array);
                    _exit(1);
                }
                

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
            fprintf(stderr,"Error: Cannot open file %s.\n", file);
            _exit(1);
        }
    int exit_found = 0;
    char buf[512];
    while ((fgets(buf, sizeof(buf), fp) != NULL) && (exit_found == 0)){
        const char delim[2] = " ";
        char **array = malloc(64);
        buf[strcspn(buf, "\n")] = 0;
        char *token = strtok(buf, delim);
        int length = 0;
        while(token != NULL) {
            if (!isEmpty(token)) {
                array[length] = malloc(sizeof(token) + 1);
                Kcopy(token, array[length]);
                length++;
            }
            
            token = strtok(NULL, delim);
        }
        array[length] = NULL;
        for(int i = 0; i < length; i++){
            write(1, array[i], strlen(array[i]));
            if(i != length - 1){
                write(1, " ", 1);
            }else{
                write(1, "\n", 1);
            }
        }
       // printf("in herer1\n");
        if(isEmpty(array[0])){
            continue;
        }
        
        if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            //Is array[2] catching all the arguments
            store_alias(array[1], array);
            write(1, "alias detected\n", 16);
        }else if(strncmp(array[0], "unalias",512) == 0){
            unalias(array[1], head);
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
                    write(2, "Redirection misformatted.\n", 26);
                    continue;
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
                    

                    if(isEmpty(file_name)){
                        fprintf(stderr, "Redirection misformatted.\n");
                        exit(1);
                        //continue;
                    }else{
                        //int saved_stdout = dup(1);
                        //close(STDOUT_FILENO);
                        int desc = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                        if(dup2(desc, STDOUT_FILENO) < 0) {
                        //Is this the correct error?
                            printf("Cannot write to file %s.\n", file_name);
                        }
                        close(desc);
                        execute(array);
                       
                        //dup2(saved_stdout, 1);
                        //close(saved_stdout);
                        _exit(1);
                    }
                    
                }else{
                   execute(array);
                    _exit(1);
                }
                

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
        /*
        for(int i = 0; i < length; i++){
                free(array[i]);
        }
        free(array);
        free(token);
        */
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
        write(STDERR_FILENO, "Usage: mysh [batch-file]\n", 25);
        exit(1);
    }
  return 0;
}
