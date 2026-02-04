#include "loop.h"
#include "parse.h"
#include "proc.h"
#include "log.h"
#include "redis.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../stats_c.h"

extern const char *stats_file;
extern int stats_interval_min;

void main_loop(char *hostname, int port, int timeout, char *bootfile) {
    eater_ctx c;
    input_params_ctx ictx;
    redis_ctx rctx;
    char ibuff[INPUT_BUFFER_SIZE], obuff[INPUT_BUFFER_SIZE];
    char *b;
    int n, a;
    char **argv, *rsp;
    int argc, *argvlen;
    char debug[4000];
    stats_c_t stats;
    stats_c_init(&stats, "redisgw", stats_file, stats_interval_min);
    uint64_t stats_start_ms = 0;
    int stats_pending = 0;
    
    eater_init(&c);
    processor_init(&ictx);
    redis_init(&rctx, hostname, port, timeout);
    
    if (bootfile) {
      char *s, *e;
      int argvlenbuf[512], argvbuf[512];
      int bf, rc, next_line;

      bf = open(bootfile, O_RDONLY);
      if (bf < 0) {
        write_error("failed openning redis boot file %s: %s", bootfile, strerror(errno));
        exit(-1);
      }

      rc = read(bf, &ibuff, sizeof(ibuff)-1);
      if (rc < 0) {
        write_error("failed reading redis boot file %s: %s", bootfile, strerror(errno));
        exit(-1);
      }
      ibuff[rc] = '\0';
      if (strlen(ibuff) == sizeof(ibuff)-1) {
        write_error("bootfile %s too large to apply", bootfile);
        exit(-1);
      }

      close(bf);

      argc = 0;
      s = e = ibuff;
      argv = (char **)&argvbuf;
      argvlen = (int *)&argvlenbuf;
      while(*e) {
        if (*e == '\n') {
          *e = '\0';
          argv[argc] = s;
          argvlen[argc] = strlen(s);
          e++;
          s = e;
          if (argvlen[argc]) {
            write_debug("collected arg #%d: %s", argc, argv[argc]);
            argc++;
          } else {
            write_debug("calling redis server");
            redis_process(&rctx, argc, argv, argvlen,
                          obuff, sizeof(obuff));
            write_info("redis initialization response: %s", obuff);
            argc = 0;
          }
        } else {
          e++;
        }
      }

      if (s != e) {
        write_warning("possible garbage at the end of redis boot file %s", bootfile);
      }
    }

    for(;;) {
        n = read(0, ibuff, sizeof(ibuff));
        log_escape_string(ibuff, n, debug, 4000);
        write_debug("raw stdin: [%s]", debug);
        if (n == 0) {
            write_error("stdin closed");
            break;
        }
        b = ibuff;
        for(;;) {
            a = eater_uppend(&c, b, n);
            if (c.error_code == ER_PAIR_READY) {
                if (!stats_pending) {
                    stats_start_ms = stats_c_now_ms();
                    stats_pending = 1;
                }
                processor_append(&ictx, &c);
            } else if (c.error_code == ER_COMPLETE) {
                processor_pop(&ictx, &argc, &argv, &argvlen);
                redis_process(&rctx, argc, argv, argvlen, 
                              obuff, sizeof(obuff));
                write(1, obuff, strlen(obuff));
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
