#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "args.h"

#define BUFF_SIZE 4096

#define ALL   100
#define DEBUG   5
#define INFO    4
#define WARNING 3
#define STAT    2
#define ERROR   1
#define QUIET   0

void log_escape_string(char   *in_buff,
                       ssize_t in_buff_size,
                       char   *out_buff,
                       ssize_t out_buff_size);

void logging_init();
void set_logging_level(int);

void write_error(char *fmt, ...);
void write_stat(char *fmt, ...);
void write_warning(char *fmt, ...);
void write_info(char *fmt, ...);
void write_debug(char *fmt, ...);

#endif /* __LOG_H */
