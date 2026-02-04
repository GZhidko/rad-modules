#include "log.h"
#include "args.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char *stats_file = NULL;
int stats_interval_min = 5;
int no_external = 0;

int _process_args(int argc, char *argv[], int *debug_level,
                  char **hostname, int *port, int *timeout, char **bootfile) {
    int i;
    char *c;

    if (argc == 1) {
        return 1;
    }

    *port = 6379;
    *timeout = 1500000;

    *debug_level = 1;
    *bootfile = NULL;
    for (i=1; i<argc; i++) {
        c = argv[i];
        if (*c != '-') {
            return 1;
        }
        c++;
        switch(*c) {
        case 'd':
          (*debug_level)++;
            break;
        case 'q':
            *debug_level = 0;
            break;
        case 's':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
                *hostname = argv[i];
            else {
                printf("redis server hostname is missing\n");
                return -1;
            }
            break;
        case 'S':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
                stats_file = argv[i];
            else {
                printf("stats file is missing\n");
                return -1;
            }
            break;
        case 'p':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
                *port = atoi(argv[i]);
            else {
                printf("redis server port is missing\n");
                return -1;
            }
            break;
        case 't':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
                *port = atoi(argv[i]);
            else {
                printf("redis server timeout is missing\n");
                return -1;
            }
            break;
        case 'i':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
              *bootfile = argv[i];
            else {
                printf("redis initial commands file is missing\n");
                return -1;
            }
            break;
        case 'I':
            i++;
            if (i<argc && argv[i] && argv[i][0] != '-')
              stats_interval_min = atoi(argv[i]);
            else {
                printf("stats interval is missing\n");
                return -1;
            }
            break;
        case 'n':
            no_external = 1;
            break;
        default:
            printf("bad option\n");
            return 1;
        }
    }
    return 0;
}

void process_args(int argc, char *argv[], int *debug_level, 
                  char **hostname, int* port, int *timeout, char **bootfile) {
  if (_process_args(argc, argv, debug_level, hostname, port, 
                    timeout, bootfile)) {
        printf("" PACKAGE_STRING "\n"
               "Usage: " PACKAGE " -[h]\n"
               "                          -d\n"
               "                          -s address\n"
               "                          -p port\n"
               "                          -t timeout\n"
               "                          -i redis initial commands\n"
               "                          -S statfile\n"
               "                          -I minutes\n"
               "                          -n (no external)\n");
        exit(1);
  }
}
