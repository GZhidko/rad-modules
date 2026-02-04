#include "log.h"

int process_args(int argc, char *argv[]) {
  extern int debug;
  extern char **argv_files;
  int i;
  for (i=1; i<argc; i++) {
    if (strcmp("-d", argv[i]) == 0) debug = 1;
    else if (strcmp("-s", argv[i]) == 0) {
      write_error("ascfile: warning: -s flag no longer has any effect\n");
    } else if (strcmp("-h", argv[i]) == 0) {
      return 0;
    } else {
      argv_files  = argv + i;
      return 1;
    }
  }
  return 0;
}
