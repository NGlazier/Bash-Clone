/*
 * Ostermann's shell header file
 */


#define MAX_ARGS 100


/* This is the definition of a command */
struct command {
    char *command;  //command name
    int argc;  //argument count
    char *argv[MAX_ARGS]; //argument vector
    char *infile; //input file name
    char *outfile; // output file name
    char *errfile;  //error file name

    char output_append;		/* boolean: append stdout? */
    char error_append;		/* boolean: append stderr? */

    struct command *nextpipe; //linked list pointer
};


/* externals */
extern int yydebug;


/* global routine decls */
void doline(struct command *pass, int lines);
int yyparse(void);
void catch_ctrl_c(int sig_num);//exit message
