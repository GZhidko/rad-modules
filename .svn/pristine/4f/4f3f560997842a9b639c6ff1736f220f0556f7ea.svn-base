#include <stdio.h>
#include <string.h>
#include <time.h>
/* Linux */
#include <sys/types.h>
#include <unistd.h>
/* /Linux */

#include "log.h"
#include "proc.h"

int log_level = INFO;

pid_t proc_pid = 0;

char proc_mode = '?';

char *log_escape_string_table[] = {
    "\\x00", "\\x01", "\\x02", "\\x03", "\\x04", "\\x05", "\\x06", "\\x07",
    "\\x08", "\\x09", "\\x0a", "\\x0b", "\\x0c", "\\x0d", "\\x0e", "\\x0f",
    "\\x10", "\\x11", "\\x12", "\\x13", "\\x14", "\\x15", "\\x16", "\\x17",
    "\\x18", "\\x19", "\\x1a", "\\x1b", "\\x1c", "\\x1d", "\\x1e", "\\x1f",
    " ",     "!",     "\\x22", "#",     "$",     "%",     "&",     "'",
    "(",     ")",     "*",     "+",     ",",     "-",     ".",     "/",
    "0",     "1",     "2",     "3",     "4",     "5",     "6",     "7",
    "8",     "9",     ":",     ";",     "<",     "=",     ">",     "?",
    "@",     "A",     "B",     "C",     "D",     "E",     "F",     "G",
    "H",     "I",     "J",     "K",     "L",     "M",     "N",     "O",
    "P",     "Q",     "R",     "S",     "T",     "U",     "V",     "W",
    "X",     "Y",     "Z",     "[",     "\\x5c", "]",     "^",     "_",
    "`",     "a",     "b",     "c",     "d",     "e",     "f",     "g",
    "h",     "i",     "j",     "k",     "l",     "m",     "n",     "o",
    "p",     "q",     "r",     "s",     "t",     "u",     "v",     "w",
    "x",     "y",     "z",     "{",     "|",     "}",     "~",     "\\x7f",
    "\\x80", "\\x81", "\\x82", "\\x83", "\\x84", "\\x85", "\\x86", "\\x87",
    "\\x88", "\\x89", "\\x8a", "\\x8b", "\\x8c", "\\x8d", "\\x8e", "\\x8f",
    "\\x90", "\\x91", "\\x92", "\\x93", "\\x94", "\\x95", "\\x96", "\\x97",
    "\\x98", "\\x99", "\\x9a", "\\x9b", "\\x9c", "\\x9d", "\\x9e", "\\x9f",
    "\\xa0", "\\xa1", "\\xa2", "\\xa3", "\\xa4", "\\xa5", "\\xa6", "\\xa7",
    "\\xa8", "\\xa9", "\\xaa", "\\xab", "\\xac", "\\xad", "\\xae", "\\xaf",
    "\\xb0", "\\xb1", "\\xb2", "\\xb3", "\\xb4", "\\xb5", "\\xb6", "\\xb7",
    "\\xb8", "\\xb9", "\\xba", "\\xbb", "\\xbc", "\\xbd", "\\xbe", "\\xbf",
    "\\xc0", "\\xc1", "\\xc2", "\\xc3", "\\xc4", "\\xc5", "\\xc6", "\\xc7",
    "\\xc8", "\\xc9", "\\xca", "\\xcb", "\\xcc", "\\xcd", "\\xce", "\\xcf",
    "\\xd0", "\\xd1", "\\xd2", "\\xd3", "\\xd4", "\\xd5", "\\xd6", "\\xd7",
    "\\xd8", "\\xd9", "\\xda", "\\xdb", "\\xdc", "\\xdd", "\\xde", "\\xdf",
    "\\xe0", "\\xe1", "\\xe2", "\\xe3", "\\xe4", "\\xe5", "\\xe6", "\\xe7",
    "\\xe8", "\\xe9", "\\xea", "\\xeb", "\\xec", "\\xed", "\\xee", "\\xef",
    "\\xf0", "\\xf1", "\\xf2", "\\xf3", "\\xf4", "\\xf5", "\\xf6", "\\xf7",
    "\\xf8", "\\xf9", "\\xfa", "\\xfb", "\\xfc", "\\xfd", "\\xfe", "\\xff" };

void log_escape_string(char   *in_buff,
                       ssize_t in_buff_size,
                       char   *out_buff,
                       ssize_t out_buff_size) {
    char *in_end = in_buff + in_buff_size;
    char *out_end = out_buff + out_buff_size - 4;
    char *s;
    char *p;
    char *q = out_buff;
    if (out_buff_size == 0) return;
    if (out_buff_size >  5) {
        for (p=in_buff; p<in_end; p++) {
            s = log_escape_string_table[(int)*p & 255];
            for (; *s; s++) {
                *q++ = *s;
                if (q > out_end) {
                    *q++ = '.';
                    *q++ = '.';
                    *q++ = '.';
                    *q = '\0';
                    return;
                }
            }
        }
    }
    *q = '\0';
}

void logging_init(int mode)
{
    if (mode == PROCESSOR_MODE_ENCODE) {
        proc_mode = 'e';
    } else {
        proc_mode = 'd';
    }
    proc_pid = getpid();
}

void set_logging_level(int level)
{
    log_level = level;
}

void universal_write(char * mode, char * fmt, va_list ap)
{
    char s[BUFF_SIZE];
    char o[BUFF_SIZE];
    time_t stamp;
    char *stamp_p;
    char stamp_buff[26];
    
    stamp = time(&stamp);
    stamp_p = asctime(localtime(&stamp));
    memcpy(stamp_buff, stamp_p, 25);
    stamp_buff[24] = 0;
    
    vsnprintf(s, BUFF_SIZE, fmt, ap);
    snprintf(o, BUFF_SIZE, "%s [%s] [%c] [%d] [%s] %s\n", 
             stamp_buff, PACKAGE_NAME, proc_mode, (int)proc_pid, mode, s);
    write(2, o, strlen(o));
}

void write_error(char *fmt, ...)
{
    va_list ap;
    if (log_level > 0) {
        va_start(ap, fmt);
        universal_write("error", fmt, ap);
        va_end(ap);
    }
}

void write_stat(char *fmt, ...)
{
    va_list ap;
    if (log_level > 1) {
        va_start(ap, fmt);
        universal_write("stat", fmt, ap);
        va_end(ap);
    }
}

void write_warning(char *fmt, ...)
{
    va_list ap;
    if (log_level > 2) {
        va_start(ap, fmt);
        universal_write("warning", fmt, ap);
        va_end(ap);
    }
}

void write_info(char *fmt, ...)
{
    va_list ap;
    if (log_level > 3) {
        va_start(ap, fmt);
        universal_write("info", fmt, ap);
        va_end(ap);
    }
}

void write_debug(char *fmt, ...)
{
    va_list ap;
    if (log_level > 4) {
        va_start(ap, fmt);
        universal_write("debug", fmt, ap);
        va_end(ap);
    }
}
