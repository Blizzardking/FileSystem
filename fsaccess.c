#include "common.h"
#include "v6util.h"
#define MAX 100


void tokenize(char *string, char *token_list[], int *argc);
//void fork_execute(const char *path, char *const argv[]);
//int status;

char seps[] = " ,\t\n";

void usage() {
    printf("initfs file_name(representing disk) n1(total number of blocks) n2(total number of blocks containing inodes)\n");
    printf("cpin external_file_name v6_file\n");
    printf("cpout v6_file external_file_name\n");
    printf("mkdir V6-directory\n");
    printf("cd [DIR]\n");
    printf("ls\n");
}

int main() {
    usage();
    while(1) {
        char string[MAX];
        fgets(string, MAX, stdin);
        int length = strlen(string) - 1;
        string[length] = '\0';
        if(strcmp(string, "q") == 0){
           break; 
        }
        char* argv[5];
        int argc;
        tokenize(string, argv, &argc);
        if(argv[0] != '\0') { 
            if(strcmp(argv[0], "cpin") == 0)
                cpin(argc, argv);
            else if(strcmp(argv[0], "initfs") == 0)
                initfs(argc,argv);
            else if(strcmp(argv[0], "cpout") == 0)
                cpout(argc,argv);
            else if(strcmp(argv[0], "mkdir") == 0)
                mkdir1(argc,argv);
            else if(strcmp(argv[0], "ls") == 0)
                ls(argc,argv);
            else if(strcmp(argv[0], "cd") == 0)
                cd(argc, argv);
            else {
                usage();
            }
        }
     }
     close(curr_fd);
     return 0;
}


void tokenize(char *string, char *token_list[], int *argc)
{
    
    /* Establish string and get the first token: */
    char * token;
    *argc = 0;
    token = strtok(string, seps );
    while( token != NULL )
    {
        /* While there are tokens in "string" */
        token_list[*argc] = token;
        (*argc)++;
        /* Get next token: */
        token = strtok( NULL, seps );
    }
    token_list[(*argc)] = (char*)0; 
}



/*
void fork_execute(const char *path, char *const argv[]){
    int child_pid; 
    if((child_pid = fork()) < 0 )
    {
        perror("fork failure");
        exit(-1);
    }    
    if (child_pid == 0 ) {
        if(execvp(path, argv) != 0) {
            printf("%s faulure\n", argv[0] );
        }
        exit(0);
    }
    else {
        printf("%s are executed!\n", argv[0] );
        if (wait(&status) == -1)
            perror("wait error");                
    }
}
*/
