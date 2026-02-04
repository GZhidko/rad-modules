#include "loop.h"
#include "arg.h"

int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  main_loop();
  return 0; /* XXX NEVER REACH */
}
