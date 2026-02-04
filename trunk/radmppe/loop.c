#include "loop.h"
#include "parse.h"
#include "proc.h"
#include "log.h"
#include <stdlib.h>
#include "../stats_c.h"

extern const char *stats_file;
extern int stats_interval_min;

void main_loop(int mode) {
    eater_ctx c;
    input_params_ctx ictx;
    char buff[INPUT_BUFFER_SIZE];
    char * b;
    int n, a;
    char debug[4000];
    stats_c_t stats;
    stats_c_init(&stats, "radmppe", stats_file, stats_interval_min);
    uint64_t stats_start_ms = 0;
    int stats_pending = 0;
    
    eater_init(&c);
    processor_init(&ictx, mode);

    for(;;) {
        n = read(0, buff, INPUT_BUFFER_SIZE);
        log_escape_string(buff, n, debug, 4000);
        write_debug("raw stdin: [%s]", debug);
        if (n == 0) {
            write_error("stdin closed");
            break;
        }
        b = buff;
        for(;;) {
            a = eater_uppend(&c, b, n);
            if (c.error_code == ER_PAIR_READY) {
                if (!stats_pending) {
                    stats_start_ms = stats_c_now_ms();
                    stats_pending = 1;
                }
                processor_append(&ictx, &c);
            } else if (c.error_code == ER_COMPLETE) {
                processor_process(&ictx);
                if (stats_pending) {
                    stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
                    stats_pending = 0;
                }
            } else if (c.error_code != ER_UNDEF) {
                eater_dump(&c);
                exit(1);
            }
            n -= a;
            if (n == 0) {
                break;
            }
            b += a;
        }
    }
}
