#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdbool.h>

int isEmpty(char* phrase){
    int length = strlen(phrase);
    for(int i = 0; i< length; i++){
        if(!isspace(phrase[i])){
            return 0;
        }
    }
    return 1;
}

// void Kcopy(char* from, char* to){
//     int j = 0;
    
//     for (j = 0; from[j] != '\0'; ++j) {
//         to[j] = from[j];
//     }
//     to[j] = '\0';
// }

int Ktrim(char* string){
    int counter = 0;
    for (int i = 0; string[i] != '\0'; ++i) {
        if(!isspace(string[i])){
            string[counter] = string[i];
            counter++;
        }else{
            if(!isspace(string[i+1])){
                string[counter] = ' ';
                counter++; 
            }
        }
    }
    string[counter] = '\0';
    return counter;
}


struct node {
    struct node* child;
    char name[512];
    char **arg;
};

struct node* head = NULL;

bool search_alias(char *name, struct node *curr, char ***returnArray){
    if(head == NULL){
        return false;
    }
   // printf("curr name: %s, name: %s\n", curr->name, name);
    if(strcmp(curr->name, name) == 0){
        *returnArray = curr->arg;
        return true;
    }
    
    if(curr-> child == NULL){
        return false;
    }

    return search_alias(name, curr->child, returnArray);

}

void store_alias(char *name, char** val){
    char** emptyArray;
    
    if(isEmpty(name)){
        return;
    }

    if(search_alias(name,head, &emptyArray)){
        return;
    }

    char danger1[] = "alias";
    char danger2[] = "unalias";
    char danger3[] = "exit";

    if((strcasecmp(danger1, name)==0) || (strcasecmp(danger2, name)==0) || (strcasecmp(danger3, name)==0)){
        return;
    }
    struct node *addNode = (struct node*) malloc(sizeof(struct node));
    addNode->arg = malloc(16 * sizeof(char**));
    addNode->child = NULL;
    
    strcpy(addNode->name, name);
    
    int j = 0;
    int i = 2;
    while (val[i] != NULL) {
        //char *cpy = addNode->arg[i];
        addNode->arg[j] = malloc(strlen(val[i]) + 1);
        strcpy(addNode->arg[j], val[i]);
        i++;
        j++;
    }
    if(head!= NULL){
        addNode->child = head->child;
    }
    addNode->arg[j] = NULL;

    struct node *old = head;
    head = addNode;
    head->child = old;
    fflush(stdout);
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

    return 1;
}

void printAlias(struct node *head){
    while(head != NULL) {
        printf("%s ", head->name);
        int i =0;
        
        while( head->arg[i] != NULL){
            printf(" %s", head->arg[i]);
            i++;
        }

        printf("\n");
        if (head->child != NULL) {
            head = head->child;
        } else {
            head = NULL;
            continue;
        }
    }
}

void onePrint(char *name, struct node *head){
    if(head == NULL){
        return;
    }
    
    if(strcmp(head->name, name) == 0){
        printf("%s ", head->name);
        int i =0;
        
        while( head->arg[i] != NULL){
            printf(" %s", head->arg[i]);
            i++;
        }

        printf("\n");
        return;
    }
    
    // if(head-> child == NULL){
    //     return ;
    // }

    onePrint(name, head->child);

    return;
}


void execute(char** array){
    if(!isEmpty(array[0])){
        char** aliasArray;
        bool check = search_alias(array[0], head, &aliasArray);
        //printf("arg: %s %s\n", aliasArray[0], aliasArray[1]);
        if(check){
            //call exec with alias
            execv((char*)aliasArray[0], aliasArray);
            fprintf(stderr, "%s: Command not found.\n", array[0]);
        }else{     
            execv(array[0], array);
            fprintf(stderr, "%s: Command not found.\n", array[0]);  
        }
    }
}

void filter(char** array, int length){
    int arrayIndex = -1; //index in array where < is found
    int position = -1; //index inside of string where < is found
    int counter = 0;
    int flength = length;
    const char comparison[] = ">";
    char* token;
    for(int i = 0; i < length; i++){
        int temp = strcspn(array[i],comparison);
        if(temp < strlen(array[i])){
                counter++;
                arrayIndex = i;
                position = temp;
        }
    }
    char file_name[512];
    char **tempArray = malloc(64 * sizeof(char **));
    char *temp = malloc(sizeof(array[arrayIndex]));
    int det = 0;
    if(counter != 0){
         det = 1;
        // // more than one > 
        // // there is nothing after > 
        // // there is something after > and another array position
        // // > not in last two positions
         if (counter > 1 
             || (position == strlen(array[arrayIndex]) - 1 && arrayIndex == length - 1) 
             || (position != strlen(array[arrayIndex]) - 1 && arrayIndex != length - 1)
             || (arrayIndex < length - 2)){
             write(2, "Redirection misformatted.\n", 26);
             return;
         }
         strcpy(temp,array[arrayIndex]);
        // //There is something after > in the same string
         if(position != strlen(array[arrayIndex]) - 1){
            if(position != 0){
                token = strtok(temp, comparison);
                token = strtok(NULL, comparison);
                strcpy(file_name, token);
                strcpy(array[arrayIndex], temp);
                //A>B
            }else{
                token = strtok(temp, comparison);
                strcpy(file_name, token);
                flength--;
                //A >B
            }
         }else{
            strcpy(file_name, array[arrayIndex + 1]);
            if(position != 0){
                int j = 0;
                for (j = 0; temp[j] != '>'; ++j) {
                    array[arrayIndex][j] = temp[j];
                }
                array[arrayIndex][j] = '\0';
                flength = length - 1;
                //A> B
            }else{
                flength = length - 2;
                //A > B
            }
         }
         
    }
    for(int i = 0; i < flength; i++){
        tempArray[i] = malloc(strlen(array[i]) + 1);
        strcpy(tempArray[i], array[i]);
    }
    tempArray[flength] = NULL;
    pid_t pid = fork();
    int status;
    if(pid == 0){
        //Child
        if(det == 1){
            if(isEmpty(file_name)){
                fprintf(stderr, "Redirection misformatted.\n");
                exit(1);
            }else{
                int desc = open(file_name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if(dup2(desc, STDOUT_FILENO) < 0) {
                    //Is this the correct error?
                    printf("Cannot write to file %s.\n", file_name);
                }
                close(desc);
                execute(tempArray);
                _exit(1);
            }
        }else{
            execute(tempArray);
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

    for(int i = 0; i < flength; i++){
             free(tempArray[i]);
    }
    free(temp);
    free(tempArray);
}


void turtle_mode(){
    int exit_found = 0;
    do{
        const char delim[2] = " ";
        char buf[512];
        char **array = malloc(64* sizeof(char *));
        write(1, "mysh> ", 6);
        if(fgets(buf, sizeof buf, stdin) == NULL){
            exit_found = 1;
            continue;
        }
        
        buf[strcspn(buf, "\n")] = 0;
        int check = Ktrim(buf);
        if(check == 0){
            continue;
        }
        char* token = strtok(buf, delim);
        int length = 0;
        while(token != NULL) {
            if (!isEmpty(token)) {
                array[length] = malloc(strlen(token) + 1);
                strcpy(array[length], token);
                length++;

            }

            token = strtok(NULL, delim);
        }
        array[length] = NULL;
        if(strlen(buf) == 0){
        }else if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            //write(1, "alias detected\n", 16);
            if(array[2]!=NULL){
                store_alias(array[1], array);
            }else if(array[1]!=NULL){
                onePrint(array[1], head);
            }else{
                printAlias(head);
            }

        }else if(strncmp(array[0], "unalias",512) == 0){
            write(1, "unalias detected\n", 18);
            unalias(array[1], head);
        }else{
            filter(array, length); 
        }


        for(int i = 0; i < length; i++){
             free(array[i]);
        }
        free(array);
        //free(token); 
    }while(!exit_found);
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
    while ((fgets(buf, sizeof(buf), fp) != NULL) && (!exit_found )){
        
        const char delim[2] = " ";
        char **array = malloc(64 * sizeof(char **));
        buf[strcspn(buf, "\n")] = 0;
        int check = Ktrim(buf);
        
        if(check == 0){
            printf("%s\n", buf);
            continue;
        }

        char *token = strtok(buf, delim);
        int length = 0;
        
        while(token != NULL) {
            if (!isEmpty(token)) {
                array[length] = malloc(strlen(token) + 1);
                strcpy(array[length], token);
                array[length][strlen(token)] = '\0';
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

       
        if(isEmpty(array[0])){
            continue;
        }
        
        if(strncmp(array[0], "exit",512) == 0){
            exit_found = 1;
        }else if(strncmp(array[0], "alias",512) == 0){
            //Is array[2] catching all the arguments
            if(array[2]!=NULL){
                store_alias(array[1], array);
            }else if(array[1]!=NULL){
                onePrint(array[1], head);
            }else{
                printAlias(head);
            }
            
            //write(1, "alias detected\n", 16);
        }else if(strncmp(array[0], "unalias",512) == 0){
            unalias(array[1], head);
            write(1, "unalias detected\n", 18);
        }else{
            filter(array, length); 
        }
        for(int i = 0; i < length; i++){
             free(array[i]);
        }
        free(array);
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
