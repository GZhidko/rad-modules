%option stack

%{
#include <string.h>
#include "log.h"
#define QBUFFSIZE 4096
char *quoted_text;
char *buffer;
char *active_key;
char **argv_files;
int  has_eq;
int  has_buffer;
void append_quoted();
void init_quoted();
void eat_word(char *);
void eat_key(char *);
void eat_attribute_and_value(char *, char *);
void eat_only_value(char *);
%}

%x INLINE
%x QUOTED

ORD_CHARS [^\0-\40\176-\377,"=\\#]
EXT_CHARS [^\0-\40\176-\377"\\]

%%

<INITIAL,INLINE>{
[[:blank:]]*#.* ;
\n+             BEGIN INITIAL;
=               has_eq = 1;
\"              init_quoted(); yy_push_state(QUOTED);
}
<INITIAL>{
[[:blank:]]+    BEGIN INLINE;
{ORD_CHARS}+    eat_word(yytext); BEGIN INLINE;
}
<INLINE>{
[[:blank:],]+   ;
{ORD_CHARS}+    eat_word(yytext);
}
<QUOTED>{
{EXT_CHARS}+    append_quoted();
\\[\\\"]        append_quoted();
\"              append_quoted(); yy_pop_state(); eat_word(quoted_text);
}

%%

void append_quoted() {
  strcat(quoted_text, yytext);   /* XXX space */
}

void init_quoted() {
  *quoted_text = '\0';
  strcat(quoted_text, yytext);
}

void eat_word(char *s) {
/*  printf("\033[32m[%d:%d:%s]\033[0m\n", has_eq, has_buffer, s); */
  if (YY_START == INITIAL) {
    if (has_buffer) {
      eat_only_value(buffer);
    }
    eat_key(s);
    has_buffer = 0;
  } else {
    if (has_eq) {
      if (has_buffer) {
        eat_attribute_and_value(buffer, s);
	has_buffer = 0;
      } else {
        write_error("Parse error: incorrect '='\n");
	exit(1);
      }
    } else {
      if (has_buffer) {
        eat_only_value(buffer);
      } else {
        has_buffer = 1;
      }
      strcpy(buffer, s);
    }
  }
  has_eq = 0;
}

void eat_key(char *s) {
  strcpy(active_key, s);
}

void eat_only_value(char *s) {
  char *attribute;
  char *p;
  int dig = 0;
  int dot = 0;
  int oth = 0;
  for (p=s; *p; p++) {
    if        (*p == '.'             ) { dot++;
    } else if (*p >= '0' && *p <= '9') { dig++;
    } else                             { oth++;
    }
  }
  if        (oth     ) { attribute = "str";
  } else if (dot == 3) { attribute = "ip" ;
  } else if (dot == 0) { attribute = "int";
  } else               { attribute = "str";
  }
  eat_attribute_and_value(attribute, s);
}

void eat_attribute_and_value(char *a, char *v) {
  char buff[QBUFFSIZE];
  sprintf(buff, "%s = %s\n", a, v);
  reg_record(active_key, buff);
}

/** UTIL **/

void wrap_open(char *name) {
  extern int debug;
  if (debug) {
    write_error("*** OPENING FILE: %s\n", name);
  }
  yyin = fopen(name, "r");
  if (yyin == NULL) {
    write_error("Can not open file %s.\n", name);
    exit(1);
  }
}

int yywrap() {
  if (has_buffer) {
    eat_only_value(buffer);
  }
  argv_files ++;
  if (*argv_files == NULL) return 1;
  wrap_open(*argv_files);
  return 0;
}

void init_lex() {
  wrap_open(*argv_files);
  quoted_text = malloc(QBUFFSIZE);
  buffer      = malloc(QBUFFSIZE);
  active_key  = malloc(QBUFFSIZE);
  has_buffer  = 0;
  has_eq      = 0;
}

void free_lex() {
  free(quoted_text);
  free(buffer);
  free(active_key);
}

