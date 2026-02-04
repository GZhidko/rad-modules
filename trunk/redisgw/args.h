#ifndef __ARGS_H
#define __ARGS_H

extern int no_external;

void process_args(int argc, char *argv[], int * debug_level,
                  char **hostname, int *port, int *timeout, char **bootfile);

#endif
