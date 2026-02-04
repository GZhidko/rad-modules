//#include <stdio.h>
//#include <stdlib.h>
#include "args.h"
#include "loop.h"
#include "log.h"

int main(int argc, char *argv[]) {

    int debug_level, mode;

    process_args(argc, argv, &debug_level, &mode);

    logging_init(mode);
    set_logging_level(debug_level);
    main_loop(mode);

    return 0;
}
