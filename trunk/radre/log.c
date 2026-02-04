#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "log.h"


int debug = 0;
char *app_name = "[name not set yet]";
pid_t app_pid = 0;


void setup_logger(char *name) {
  app_pid = getpid();
  app_name = name;
}


void set_debug(int d) {
  debug = d;
}


void write_error(char *fmt, ...) {
  time_t stamp;
  va_list ap;
  int bol;
  char s[1024];
  char p[1024];
  char string[2048];
  char *sp, *a, *b;
  stamp = time(&stamp);
  strcpy(p, asctime(localtime(&stamp)));
  sprintf(p + strlen(p) - 1, " %s[%u] ", basename(app_name), app_pid);
  va_start(ap, fmt);
  vsprintf(s, fmt, ap);  /* XXX */
  va_end(ap);
  bol = 1; /* bol -- begin of line flag */
  for (a=s, sp=string; *a;) {
    if (bol) {
      for (b=p; *b;) *sp++ = *b++;
      bol = 0;
    }
    if (*a == '\n') bol = 1;
    *sp++ = *a++;
  }
  *sp = '\0';
  write(2, string, strlen(string));
}
