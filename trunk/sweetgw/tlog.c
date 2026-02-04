/*
 * Application-specific logging module used by sweetuam.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <time.h>

int sw_log_init(char *filename) { return 0; }

#ifdef __STDC__
int sw_log(const char *format, ...) {
#else
int sw_log(va_alist) va_dcl {
#endif
#ifndef __STDC__
  char *format;
#endif
  va_list pvar;
  static char b[1024];
  time_t now = time(NULL);

#ifdef __STDC__
  va_start(pvar, format);
#else
  va_start(pvar);
  format = va_arg(pvar, char *);
#endif

  sprintf(b, "sweetgw[%d]: %s", getpid(), asctime(localtime(&now)));
  b[strlen(b)-1] = ' ';

  vsnprintf(b+strlen(b), sizeof(b)-strlen(b), format, pvar);
  strncat(b, "\n", sizeof(b)-1);
  b[sizeof(b)-1] = '\0';
  va_end(pvar);

  fprintf(stderr, "%s", b);

  return 0;
}
