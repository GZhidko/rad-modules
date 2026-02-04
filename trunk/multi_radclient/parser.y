%{
#include "log.h"
#include "common.h"

#define YYSTYPE char*

attr *out, *p, *tmp;
int n;
int flag;
char acc[1024];
int line;
char *procn;

void init_output(attr*, char*);
void append(char*);

%}

%token QUOTE EQUAL TAB EOLN SP STR TP

%%
start:
| start TAB q1 {strncpy(p->value, acc, sizeof(p->value)); 
     n++; 
     *acc = '\0';
     free($2);
}
| start STR q3  {strncpy(p->name, $2, sizeof(p->name)); 
     strcpy(p->value, acc);
     n++;
     *acc='\0';
     free($2);
}
| start STR TP q2 {strncpy(p->space, $2, sizeof(p->space)); 
     strcpy(p->value, acc); 
     n++;
     *acc='\0';
     free($2);
     free($3);
}
;

q1: STR q3 {strncpy(p->name, $1, sizeof(p->name)); free($1);}
| STR TP q2 {strncpy(p->space, $1, sizeof(p->space)); free($1); free($2);}
;

q2: STR TP q4 {strncpy(p->vendor, $1, sizeof(p->vendor)); free($1); free($2);}
;

q3: SP q5 {free($1);}
| EQUAL q6 {free($1);}
;

q4: STR q3 {strncpy(p->name, $1, sizeof(p->name)); free($1);}
;

q5: EQUAL q6 {free($1);}
;

q6: QUOTE q9 {append($1);}
| SP q7 {free($1);}
| STR TP q8 {strncpy(p->type, $1, sizeof(p->type)); free($1); free($2);}
| STR q10 {append($1);}
;

q7: QUOTE q9 {append($1);}
| SP q12 {append($1);}
| STR q10 {append($1);}
| STR TP q8 {strncpy(p->type, $1, sizeof(p->type)); free($1); free($2);}
;


q8: QUOTE q9 {append($1);}
| STR q10 {append($1);}
| SP q12 {append($1);}
;

q9: STR q13 {append($1);}
| SP q12 {append($1);}
| TP q18 {append($1);}
| EQUAL q15 {append($1);}
| QUOTE q14 {append($1);}
;

q10: SP q12 {append($1);}
| EOLN  {
   *acc='\0';
    if (flag == 1) {
	 flag = 0;
     } else {
	 tmp = (attr*)malloc(sizeof(attr));
	 tmp->next = NULL;
	 init_attr(tmp);
	 p->next = tmp;
	 p = tmp;
    }
    free($1);
}
| QUOTE q14 {append($1);}
| EQUAL q15 {append($1);}
;

q12: SP q12 {append($1);}
| QUOTE q14 {append($1);}
| TP q18 {append($1);}
| EQUAL q15 {append($1);}
| STR q13 {append($1);}
;

q13: SP q12 {append($1);}
| STR q13 {append($1);}
| TP q13 {append($1);}
| EQUAL q15 {append($1);}
| QUOTE q14 {append($1);}
| EOLN q17 {append($1);}
;

q14: EOLN {
    *acc='\0';
    if (flag == 1) {
	flag = 0;
    } else {
	tmp = (attr*)malloc(sizeof(attr));
	tmp->next = NULL;
	init_attr(tmp);
	p->next = tmp;
	p = tmp;
    } 
    free($1);
 } 
;

q15: SP q12 {append($1);}
| STR q13 {append($1);}
| EQUAL q15 {append($1);}
| QUOTE q14 {append($1);}
| TP q18 {append($1);}
;

q17: STR q13 {append($1);}
;

q18: STR q13 {append($1);}
| QUOTE q14 {append($1);}
| EQUAL q15 {append($1);}
| SP q12 {append($1);}
| TP q18 {append($1);}
;

%%

void append(char* s) {
    char dest[1024];
    *dest = '\0';
    strcpy(dest, acc);
    strcpy(acc, s);
    strcat(acc, dest);
    free(s);
}

void init_output(attr *output, char *proc) {
    procn = proc;
    line = 1;
    syntax_error = 0;
    n = 0;
    *acc = '\0';
    p = output;
    flag = 1;
}

yyerror(char *s) {
    write_error("%s: syntax error at line %d", procn, line);
    syntax_error = 1;
}
