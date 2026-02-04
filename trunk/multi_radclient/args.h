#ifndef __ARGS_H
#define __ARGS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "dispatch.h"

int exp_time;  /* expire time  for request */
int stat_time;  /* time for statistics */
char *stats_file;
int stats_interval_min;
char prog[128];
char package_name[64];
char package_version[64];
char package_bugreport[64];
int num_rclnt;          /* number of radclient */
char **args;            /* arguments from command line for radclinet */

int process_args(int argc, char *argv[]);

#endif /* __ARGS_H */
