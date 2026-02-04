%{
#include "common.h"
#include "parser.h"

void start_scan(char*);
void stop_scan(void);

#define YY_INPUT(buf, result, max) { \
*buf = '\0'; \
result = get_data(buf, max); \
if (result == 0) \
      result = YY_NULL; \
}\

char *mmbuf;
int flag;
extern int line;
%}

%% 
"\""      {yylval=(int)strdup(yytext); return QUOTE;}
"="       {yylval=(int)strdup(yytext); return EQUAL;}
"\t"      {yylval=(int)strdup(yytext); return TAB;}
"\n"      {yylval=(int)strdup(yytext); line++; return EOLN;}
" "       {yylval=(int)strdup(yytext); return SP;}
":"       {yylval=(int)strdup(yytext); return TP;}
[^\"=\t\n\ :]+  { yylval=(int)strdup(yytext); return STR; }
%%

int yywrap() {
    return 1;
}

void start_scan(char *buf) {
    flag = 1;
    mmbuf = buf; 	    
    yyrestart(yyin);
    yyparse();
}

int get_data(char *buf, int max) {
    if (flag == 0) {
        return 0;
    }
    flag = 0;

    strncpy(buf, mmbuf, max);    
    return strlen(mmbuf);
}
