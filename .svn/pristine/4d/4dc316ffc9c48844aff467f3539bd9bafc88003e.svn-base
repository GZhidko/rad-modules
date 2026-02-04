#include "loop.h"
#include "arg.h"
#include "stor.h"

int main(int argc, char *argv[]) {
  char ** configs = parse_args(argc, argv);
  radre_stor_load(configs);
  main_loop();
  return 0; /* XXX NEVER REACH */
}
