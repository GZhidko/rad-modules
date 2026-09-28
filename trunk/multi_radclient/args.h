#ifndef __ARGS_H
#define __ARGS_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "dispatch.h"

extern int exp_time;  /* expire time  for request */
extern int stat_time;  /* time for statistics */
extern char *stats_file;
extern int stats_interval_min;
extern char prog[128];
extern char package_name[64];
extern char package_version[64];
extern char package_bugreport[64];
extern int num_rclnt;          /* number of radclient */
extern char **args;            /* arguments from command line for radclinet */

int process_args(int argc, char *argv[]);

#endif /* __ARGS_H */
