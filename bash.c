/*
 * Headstart for Ostermann's shell project
 *
 * Shawn Ostermann -- Sept 9, 2021
 */
 
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "bash.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h> /* define signal(), etc. */
#include <unistd.h>
#include <fcntl.h>


void catch_ctrl_c(int sig_num){/* user typed Ctrl-C */
   char *msg = "\nThanks for using my shell\n";
   if(write(1,msg, strlen(msg)) == -1){
       perror("ERROR writing");
   } /* printf may not be re-entrant! */ 
   exit(-1);
}




int
main(
     int argc,
     char *argv[])
{
    yydebug = 1;  /* turn on ugly YACC debugging */
    yydebug = 0;  /* turn off ugly YACC debugging */
    signal(SIGINT, catch_ctrl_c);
    putenv("DEBUG=");
    
    printf("NISH> ");
    fflush(stdout);
    /* parse the input file */
    yyparse();

    exit(0);
}





void
doline(
    struct command *pcmd, int lines)
{
    signal(SIGINT, catch_ctrl_c);
    char* path;
    putenv("PROMPT=NISH> ");
    char* home = strdup(getenv("HOME"));
    char* mypath = getenv("PATH");
    
    
    char *patharr[MAX_ARGS];

    //redirection
    char *outfilename;
    char *infilename;
    char *errfilename;
    int fdi,fdo,fde; //file descriptors for input, output, error
    fdi = fdo = fde = -1;
    
    

    //if command is NULL, do nothing
    while (pcmd != NULL){
        mypath = getenv("PATH");
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("debugenv: %s\n", getenv("DEBUG"));
        }
        path = strdup(mypath);
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("PATH is '%s'\n", path?path:"<undefined>");
            printf("HOME is '%s'\n", home?home:"<undefined>");
            printf("PROMPT is '%s'\n", getenv("PROMPT")?getenv("PROMPT"):"<undefined>");
        }
        
        
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        //print current line number
    printf("========== line %i =================\n", lines);
    
    if(pcmd -> argv[0] != NULL){
        //print the command and its arguments
        printf("Command name: ’%s’\n", pcmd -> command);
        for(int i = 0; pcmd -> argv[i] != NULL; i++){
            printf("argv[%i]: '%s'\n", i, pcmd -> argv[i]);
        }

        //prints the input redirection
        if(pcmd -> infile != NULL){
            printf("stdin: ’%s’\n", pcmd -> infile);
        }else{
            printf("stdin: ’<undirected>’\n");
        }

        //prints the output redirection
        if(pcmd -> outfile != NULL){
            if(pcmd -> output_append == 0){
                printf("stdout: ’%s’\n", pcmd -> outfile);
            }else{
                printf("stdout: ’%s’ (append)\n", pcmd -> outfile); //is it appended?
            }
        }else{
            printf("stdout: ’<undirected>’\n");
        }

        //prints the error redirection
        if(pcmd -> errfile != NULL){
            if(pcmd -> error_append == 0){
                printf("stderr: ’%s’\n", pcmd -> errfile);
            }else{
                printf("stderr: ’%s’ (append)\n", pcmd -> errfile); //is it appneded?
            }
        }else{
            printf("stderr: ’<undirected>’\n");
        }

    }




    }

     // file redirection
    if(pcmd -> outfile != NULL){
        outfilename = pcmd -> outfile;
        if(pcmd -> output_append == 0){
            fdo = open(outfilename, O_WRONLY | O_CREAT, 00700);
            if (fdo == -1) { perror(outfilename); exit(0); }
        }
        if(pcmd -> output_append != 0){
            fdo = open(outfilename, O_WRONLY | O_APPEND | O_CREAT, 00700);
            if (fdo == -1) { perror(outfilename); exit(0); }
            
        }
    }

    if(pcmd -> infile != NULL){
        infilename = pcmd -> infile;
        fdi = open(infilename, O_RDONLY);
        if (fdi == -1) { perror(infilename); exit(0); }
    }

    if(pcmd -> errfile != NULL){
        errfilename = pcmd -> errfile;
        if(pcmd -> error_append == 0){
            fde = open(errfilename, O_WRONLY | O_CREAT, 00700);
            if (fde == -1) { perror(errfilename); exit(0); }
            
        }if(pcmd -> error_append != 0){
            fde = open(errfilename, O_WRONLY | O_APPEND | O_CREAT, 00700);
            if (fde == -1) { perror(errfilename); exit(0); }
            
        }
    }
    
    
    
    
    


    
    //absolute command
    if(pcmd -> command[0] == '/' || (pcmd -> command[0] == '.' && pcmd -> command[1] == '/') || (pcmd -> command[0] == '.' && pcmd -> command[1] == '.' && pcmd -> command[2] == '/')){
        if (fork() == 0) {
        // child 
            if(pcmd -> infile != NULL){
                dup2(fdi, STDIN_FILENO); // bind stdin to fd 
                close(fdi);
            }
            if(pcmd -> outfile != NULL){
                dup2(fdo, STDOUT_FILENO); 
            }
            if(pcmd -> errfile != NULL){
                dup2(fde, STDERR_FILENO); // bind stdin to fd 
            }
            execv(pcmd -> command, pcmd -> argv);
            perror(pcmd -> command);
            if(pcmd -> outfile != NULL){
                close(fdo);
            }
            if(pcmd -> errfile != NULL){
                close(fde);
            }
            _exit(1);
        }
        wait(0);

    }else{
        //cd command
        if(strcmp(pcmd -> command, "cd") == 0){
            if(pcmd -> argv[1] == NULL){
                if(chdir(home) == -1){
                    perror("cannot change directory\n");
                }
            }else{
                if(chdir(pcmd -> argv[1]) == -1){
                    perror("cannot change directory\n");
                }
            }
        }else{
            
   
        char * pathtoken = strtok(path, ":");
        int pathcount = 0;
        // loop through the string to extract all other tokens
        //load all possible paths into array
        while(pathtoken != NULL ) {
            patharr[pathcount] = pathtoken;
            pathcount++;
            pathtoken = strtok(NULL, ":");
        }
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            for(int i = 0; i < pathcount; i++){
                printf("patharr[%i]: '%s'\n", i, patharr[i]);
            }   
        }


        int j;
        char* tmp = "";
        char* slash = "/";
        //find path/command with execute permission
        for(j=0; j < pathcount; j++){
            tmp = strdup(patharr[j]);
            strcat(tmp, slash);
            strcat(tmp, pcmd -> command);
            if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                printf("tmp: %s\n", tmp);
            }
            if(access(tmp, F_OK) == 0){
                if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                    printf("%s has permission\n", tmp);
                }
                
                break;
            }

        }

        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("patharr[%i]: %s\n", j, patharr[j]);
        }

        

        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("path: %s\n", patharr[j]);
            printf("cmd: %s\n", pcmd -> command);
        }

        //execute command with permitted path
        pcmd -> command = tmp;
        if(fork() == 0){
            if(pcmd -> infile != NULL){
                dup2(fdi, STDIN_FILENO); // bind stdin to fd 
                close(fdi);
            }
            if(pcmd -> outfile != NULL){
                dup2(fdo, STDOUT_FILENO); // bind stdout to fd 
            }
            if(pcmd -> errfile != NULL){
                dup2(fde, STDERR_FILENO); // bind stderr to fd 
            }
            execv(pcmd -> command, pcmd -> argv);
            perror(patharr[j]);
            if(pcmd -> outfile != NULL){
                close(fdo);
            }
            if(pcmd -> errfile != NULL){
                close(fde);
            }
            _exit(1);
        }
        wait(0);
        
    

    }
    }
        pcmd = pcmd -> nextpipe; //move to next command
        lines++; //increase line count
        printf("%s", getenv("PROMPT"));
        fflush(stdout);
    }
    
}
