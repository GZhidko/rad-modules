#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include "logger.h"

// --------------------------------------------------------------
// UTIL
// --------------------------------------------------------------

static char const * log_escape_string_table[] = {
 "\\0",
 "\\x01", "\\x02", "\\x03", "\\x04", "\\x05", "\\x06", "\\x07", "\\x08", "\\x09",
 "\n", /* -asis- */
 "\\x0b", "\\x0c",
 "\\r",
 "\\x0e", "\\x0f",
 "\\x10", "\\x11", "\\x12", "\\x13", "\\x14", "\\x15", "\\x16", "\\x17",
 "\\x18", "\\x19", "\\x1a", "\\x1b", "\\x1c", "\\x1d", "\\x1e", "\\x1f",
 " ", "!",
 "\"", /* -asis- */
 "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/",
 "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
 ":", ";", "<", "=", ">", "?", "@",
 "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
 "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
 "[",
 "\\", /* -asis- */
 "]", "^", "_", "`",
 "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
 "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
 "{", "|", "}", "~",
 "\\x7f",
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

ssize_t log_escape_string(char *in_buff, ssize_t in_buff_size,
                          char *out_buff, ssize_t out_buff_size) {
    char *in_end = in_buff + in_buff_size;
    char *out_end = out_buff + out_buff_size - 3;
    char const *s;
    char *p;
    char *q = out_buff;
    if (out_buff_size > 5) {
        for (p=in_buff; p<in_end; p++) {
            s = *(log_escape_string_table + *p);
            for (; *s; s++) {
                *q++ = *s;
                if (q > out_end) {
                    *q++ = '.';
                    *q++ = '.';
                    *q++ = '.';
                    return out_buff_size;
                }
            }
        }
    }
    return q - out_buff;
}

// --------------------------------------------------------------

Logger::Logger(OutputBuffer * b, LogLevel l) :
    data_outer(b),
    print_prefix(true),
    last_time(0),
    prefix_len(0),
    log_level(l),
    current_log_level(info),
    raw_mode(false)
{
    sprintf(prefix, " mradclient[%d]: ", getpid());
    prefix_len = strlen(prefix);
    last_time = time(&last_time);
}

// --------------------------------------------------------------

//  самый низкий уровень работы с протоколированием
//
//  минимальная обязательная функциональность:
//   -- к каждой строке добавляется дата и PID
//   -- не печатные символы котируются
void
Logger::write(char const * const data, int const len) {
    if (raw_mode) {
        data_outer->accept_data(data, len);
        print_prefix = true;
    } else {
        for (int i(0); i < len; ++i) {
//             std::cerr << "" << i << ":";
            if (print_prefix) {
                data_outer->accept_data(ctime(&last_time), 24);
                data_outer->accept_data(prefix, prefix_len);
                print_prefix = false;
            }
            // добавлять по одному символу, конечно не очень эффективно,
            // но и не так уж ужасно; тем более, это протоколирование,
            // а не основные потоки данных
            const char *s(log_escape_string_table[(unsigned char)data[i]]);
            for (; *s; s++) {
//              std::cerr << "[" << *s << "]";
                data_outer->accept_data(s, 1);
            }
            if (data[i] == '\n') {
                print_prefix = true;
            }
        }
    }
}

// обеспечение интерфейса AbstractDataAcceptor
// для удобства низкоуровневой отладки потоков;
// лучше использовать *только* во время активной разработки
void
Logger::accept_data(time_t const now, char const * const data, int const len) {
    last_time = now;
    write(data, len);
}

//
// С-стиль; быстро и сердито, лучше изжить со временем
//
void
Logger::log(LogLevel ll, time_t const now, const char * format, ...) {
    if (ll < log_level) return;

    char fout[1024];
    va_list ap;
    va_start(ap, format);
    vsnprintf(fout, 1024, format, ap);
    va_end(ap);

    accept_data(now, fout, strlen(fout));
    accept_data(now, "\n", 1);
}

// --------------------------------------------------------------

void
Logger::update_time(time_t const now) {
    last_time = now;
}

//*** ООП надо всё перевести на него ***

// операторы вывода
Logger &
Logger::operator<<(char const * c) {
    if (current_log_level >= log_level) {
        write(c, strlen(c));
    }
    return *this;
}

Logger &
Logger::operator<<(int n) {
    if (current_log_level >= log_level) {
        char c[1024]; // XXXXX use std::string / std::istringstream now
        snprintf(c, 1024, "%d", n);
        write(c, strlen(c));
    }
    return *this;
}

// аппликатор для манипулятора без аргумента (почти :-))
Logger &
Logger::operator<<(Logger & (*pf)(Logger &)) {
    return pf(*this);
}

// манипуляторы для установки уровней отладки
// использовать так
// logger << loglevel<warning>;
template<enum LogLevel E>
Logger &
loglevel(Logger & l) {
    l.current_log_level = E;
    return l;
}

template Logger & loglevel<fatal>(Logger & l);
template Logger & loglevel<error>(Logger & l);
template Logger & loglevel<warning>(Logger & l);
template Logger & loglevel<rinfo>(Logger & l);
template Logger & loglevel<info>(Logger & l);
template Logger & loglevel<debug>(Logger & l);

Logger &
endl(Logger & l) {
    return l << "\n";
}

Logger &
raw_mode(Logger & l) {
    l.raw_mode = true;
    return l;
}

Logger &
common_mode(Logger & l) {
    l.raw_mode = false;
    return l;
}
