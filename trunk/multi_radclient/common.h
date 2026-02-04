#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "log.h"

#define PACKAGE_SIZE     4096 /* max package size */
#define MAX_OPT_RCLNT    10   /* max number of radclient options */
#define MAX_OPT_SIZE     64   /* max size option for radclient */

typedef struct attr_t {
    struct attr_t *next;
    char space[64];
    char vendor[64];
    char name[64];
    char type[32];
    char value[1024];
} attr;

typedef struct second_key_t {
    int radclnt;
    int id;
    int flag;
    int time;    
} second_key;

typedef struct statistic_t {
    int request;
    int response;
    int not_response;
    int retransmit;
    int dropped;
    int time;
} statistic;

time_t global_timestamp;
int syntax_error;
statistic stat;

#endif /* __COMMON_H */
