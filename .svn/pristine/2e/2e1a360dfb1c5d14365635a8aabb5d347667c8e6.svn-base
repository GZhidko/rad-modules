#include <libgen.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void write_error(char *fmt, ...) {
  extern pid_t kvfile_pid;
  extern char *kvfile_name;
  time_t stamp;
  va_list ap;
  int bol;
  char s[1024];
  char p[1024];
  char string[2048];
  char *sp, *a, *b;
  stamp = time(&stamp);
  strcpy(p, asctime(localtime(&stamp)));
  sprintf(p + strlen(p) - 1, " %s[%u] ", basename(kvfile_name), kvfile_pid);
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
