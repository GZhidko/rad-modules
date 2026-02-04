//#include <stdio.h>
//#include <stdlib.h>
#include "args.h"
#include "loop.h"
#include "log.h"

int main(int argc, char *argv[]) {
  int debug_level, port, db, timeout;
  char *hostname, *bootfile;

  process_args(argc, argv, &debug_level, &hostname, &port,
               &timeout, &bootfile);

  logging_init();
  set_logging_level(debug_level);
  main_loop(hostname, port, timeout, bootfile);

  return 0;
}
