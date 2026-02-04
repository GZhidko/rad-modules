#include "log.h"
#include "args.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

const char *stats_file = NULL;
int stats_interval_min = 5;

int _process_args(int argc, char *argv[], int * debug_level, int * mode) {
    int i;
    char *c;
    int d;

    if (argc == 1) {
        return 1;
    }

    d = 1;
    for (i=1; i<argc; i++) {
        c = argv[i];
        if (*c != '-') {
            return 1;
        }
        c++;
        for(; *c; c++) {
            switch(*c) {
                case 'e':
                    *mode = 0;
                    break;
                case 'd':
                    *mode = 1;
                    break;
                case 'v':
                    d++;
                    break;
                case 'q':
                    d = 0;
                    break;
                case 's':
                    if (++i < argc) {
                        stats_file = argv[i];
                    } else {
                        return 1;
                    }
                    break;
                case 'i':
                    if (++i < argc) {
                        stats_interval_min = atoi(argv[i]);
                    } else {
                        return 1;
                    }
                    break;
                default:
                    return 1;
            }
        }
    }
    *debug_level = d;
    return 0;
}

void process_args(int argc, char *argv[], int * debug_level, int * mode) {
    if (_process_args(argc, argv, debug_level, mode)) {
        printf("" PACKAGE_STRING "\n"
               "Usage: " PACKAGE " -[edvq] [-s statfile] [-i minutes]\n"
               " -e -- encode mode\n"
               " -d -- decode mode\n"
               " -v -- increas debug level (up to five)\n"
               " -q -- turn off debug output\n"
               "Example: " PACKAGE " -evvv\n");
        exit(1);
    }
}
