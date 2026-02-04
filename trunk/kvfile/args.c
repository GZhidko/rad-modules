#include "log.h"
#include <stdlib.h>

const char *stats_file = NULL;
int stats_interval_min = 5;

int process_args(int argc, char *argv[]) {
  extern int debug;
  extern char **argv_files;
  int i;
  for (i=1; i<argc; i++) {
    if (strcmp("-d", argv[i]) == 0) debug = 1;
    else if (strcmp("-s", argv[i]) == 0) {
      if (i + 1 < argc) {
        stats_file = argv[++i];
      } else {
        write_error("kvfile: -s requires a path\n");
        return 0;
      }
    } else if (strcmp("-i", argv[i]) == 0) {
      if (i + 1 < argc) {
        stats_interval_min = atoi(argv[++i]);
      } else {
        write_error("kvfile: -i requires minutes\n");
        return 0;
      }
    } else if (strcmp("-h", argv[i]) == 0) {
      return 0;
    } else {
      argv_files  = argv + i;
      return 1;
    }
  }
  return 0;
}
