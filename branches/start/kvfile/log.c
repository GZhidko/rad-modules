#include <stdio.h>
#include <stdarg.h>

void write_error(char *fmt, ...) {
  va_list ap;
  char s[10240];
  va_start(ap, fmt);
  vsprintf(s, fmt, ap);  /* XXX */
  write(2, s, strlen(s));
}
