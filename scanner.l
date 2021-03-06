%{
/*
 * Simple example lex input file
 *
 * Shawn Ostermann -- Sept 9, 2021
 *
 * Headstart version
 */

#include <string.h>
#include <ctype.h>
#include "parser.h"


/* some internal files generated by bison */
void yyerror(const char *s,...);
void yywarn(const char *s,...);

int varcount = 0; //number of variables

struct vars {
    char *varname; //variable name
    char *varval;   //variable value
    int exported;   // 0 = not exported, 1 = exported
};
struct vars* vararr[100]; //array of variables


    



void varassign(char* word){
    char *varparts[2]; 
    int found = 0;
    char* eq = strtok(word, "=");
    int i = 0;
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("eq: %s\n", eq);
    }
   //split var assignment into name and value
    while(eq != NULL ) {
        varparts[i] = eq;
        i++;
        eq = strtok(NULL, "=");
    }
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("varparts[0]: %s\n", varparts[0]);
        printf("varparts[1]: %s\n", varparts[1]);
    }
    char* shortcut = strdup(varparts[0]);
    char* expanded = strdup(varparts[1]);
    //remove quotes around var value if present
    if(expanded[0] == '"'){
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("starts with a quote\n");
        }
        expanded++;
        expanded[strlen(expanded)-1] = 0;
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("expanded: %s\n", expanded);
        }
    }
    
    for(int j=0;j< varcount;j++){
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("vararr[%i]: %s\n", j, vararr[j] -> varname);
            printf("shortcut: %s\n", shortcut);
        }
        //update variable if it already exists
        if (strcmp(vararr[j] -> varname, shortcut) == 0){
            if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                printf("this variable already exists\n");
            }
            vararr[j] -> varval = expanded;
            
            if(vararr[j] -> exported == 1){
                strcat(word,"=");
                strcat(word,varparts[1]);
                if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                    printf("word: %s\n", word);
                }
                putenv(word);
            }
            found = 1;
        }
    }
    //create new variable
    if(found == 0){
        struct vars *pvars;
	pvars = (struct vars *) malloc(sizeof(struct vars));
    pvars -> varname = shortcut;
    pvars -> varval = expanded;
        if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
            printf("this is a new variable\n");
        }
        vararr[varcount] = pvars;
        varcount++;
    }
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("varcount: %i\n", varcount);
        for(int k =0;k< varcount;k++){
            printf("%s , %s\n", vararr[k] -> varname, vararr[k] -> varval);
        }
    }
}

char* varexpand(char* word){
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("time to expand %s\n", word);
    }
    //expand ${var}
    if(word[0] == '$' && word[1] == '{' && word[strlen(word) - 1] == '}'){
        word+=2;
        word[strlen(word)-1] = 0;
        for(int i =0; i < varcount; i++){
            if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                printf("we in da loop\n");
            }
            if(strcmp(vararr[i] -> varname, word) == 0){
                if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                    printf("thats a match\n");
                }
                return vararr[i] -> varval;
            }
        }
    }else{
        //expand $var
        if(word[0] == '$'){
            word++;
            for(int i =0; i < varcount; i++){
                if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                    printf("we are in the loop\n");
                }
                if(strcmp(vararr[i] -> varname, word) == 0){
                    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                        printf("thats a match\n");
                    }
                    return vararr[i] -> varval;
                }
            }
        }
    }
    return word;
   
}


/* Everything up to the funny characters on the next line */
/* goes directly into the lex.yy.c file */
%}

/* shorthand definitions for later */
DIGIT		[0123456789]
LETTER		[a-zA-Z]
OTHERCHARS	[\!_\./-@#$%^*()\[\]{}+='~;:,?`\-\\]
WHITESPACE	[ \t]
QUOTE       [\"]
SINGQUOTE   [\']
EQUAL       [=]
UNDERSCORE  [_]


/* The rest of this after the '%%' is lex rules */
%%

{WHITESPACE}+	{ }  /* Just ignore white space */

"\n" 	        { return(EOLN); }
"|"             { return(PIPE); } 
"<"             { return(INPUT); }
">"             { return(OUTPUT); }
">>"            { return(OUTPUT_APPEND); }
"2>"            { return(ERROR); }
"2>>"           { return(ERROR_APPEND); }



%{
/*a WORD can be zero or more digits, letters, special characters, or whitespace between two quotes*/
/*without quotes, a WORD can also be one or more digits, letters, or special characters*/
%}
{LETTER}+({DIGIT}|{LETTER}|{UNDERSCORE})*{EQUAL}({DIGIT}|{LETTER}|{OTHERCHARS}|{WHITESPACE})+ {
    varassign(yytext);
}

{LETTER}+({DIGIT}|{LETTER}|{UNDERSCORE})*{EQUAL}{QUOTE}({DIGIT}|{LETTER}|{OTHERCHARS}|{WHITESPACE})*{QUOTE} {
    varassign(yytext);
}

"export"" "{LETTER}+({DIGIT}|{LETTER}|{UNDERSCORE})* {
    yytext+=7; //remove "export " from word
    
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("yytext: %s\n", yytext);
    }
    //put variable in the environment
    for(int j =0; j < varcount; j++){
        if (strcmp(vararr[j] -> varname, yytext) == 0){
            vararr[j] -> exported = 1;
            char* val = strdup(vararr[j] -> varval);
            char* tmp = strdup(yytext);
            strcat(tmp, "=");
            strcat(tmp, val);
            if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
                printf("yytext to be putenv: %s\n", tmp);
            }
            putenv(tmp);
        }
    }
    if(getenv("DEBUG") != NULL && strcmp(getenv("DEBUG"), "") != 0){
        printf("put it in the environment\n");
    }
}



{QUOTE}({DIGIT}|{LETTER}|{OTHERCHARS}|{WHITESPACE})*{QUOTE}  {
    yytext++; 
    yytext[strlen(yytext)-1] = 0;
    yylval.string = strdup(yytext);
   
    return(WORD);
}

{SINGQUOTE}({DIGIT}|{LETTER}|{OTHERCHARS}|{WHITESPACE})*{SINGQUOTE}  {
    yytext++; 
    yytext[strlen(yytext)-1] = 0;
    yytext = varexpand(yytext);
    yylval.string = strdup(yytext);
   
    return(WORD);
}

({DIGIT}|{LETTER}|{OTHERCHARS})+    {
    yytext = varexpand(yytext);
    yylval.string = strdup(yytext);
    
    return(WORD);
}



.		{
    /* if we haven't matched anything yet, then it's illegal */
    fprintf(stderr, "scanner: Bad character '%c'\n", *yytext);
    yyerror("ERROR illegal character\n");
}


%%
