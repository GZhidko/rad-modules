#include <stdio.h>
#include "log.h"
#include "table.h"
#include "parser.h"
#include "loop.h"

int debug = 0;

int main(int argc, char *argv[]) {

  if (!process_args(argc, argv)) {
    printf("USAGE: ascfile -d file ...\n"
           "       ascfile -h\n");
    exit(1);
  }

  init_table();

  init_lex();
  yylex();
  free_lex();

  fix_table();

  if (debug) {
    write_error("*** DUMP OF TABLE ***\n");
    dump_table();
    write_error("*** END OF DUMP ***\n");
  }

  main_loop();

  return 0; /* XXX NEVER REACH */
}

/*** TODO:
- fix parser: correct processing non-ASCII characters
- fix parser: print cute error messages with string numbers
- clean XXX places
***/
