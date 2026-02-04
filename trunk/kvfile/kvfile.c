#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "log.h"
#include "table.h"
#include "parser.h"
#include "loop.h"

int debug = 0;
char *kvfile_name = "[name not set yet]";
pid_t kvfile_pid = 0;


int main(int argc, char *argv[]) {

  kvfile_pid = getpid();
  kvfile_name = argv[0];

  if (!process_args(argc, argv)) {
    printf("Version: 0.0.5\n"
           "Usage: kvfile [-dh] file [ file [...] ]\n");
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
