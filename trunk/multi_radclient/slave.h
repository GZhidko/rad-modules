#ifndef __SLAVE_H
#define __SLAVE_H

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include "log.h"

typedef struct slave_process {
    pid_t pid;
    int in;
    int out;
    int err;
} slave_process_t;

int slave(slave_process_t *, char *, char **);

#endif /* __SLAVE_H */
