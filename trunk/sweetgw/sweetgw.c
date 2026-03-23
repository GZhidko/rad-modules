/*
** sweetgw -- OpenRADIUS <--> Sweetspot gateway module
**
** Sends a command to Sweetspot daemon from string-type attributes,
** passed from RADIUS engine, receives a response, then builds
** a set of response attributes to be passed back to RADIUS engine.
*/

#include <sys/types.h>  /* For u(_)int32_t, htonl() */
#include <sys/socket.h> /* For u(_)int32_t, htonl() */
#include <netinet/in.h> /* For u(_)int32_t, htonl() */

#include <sys/time.h>   /* For struct timeval on some systems */
#include <time.h>   /* For struct timeval on some systems */

#include <limits.h> /* For XXX_MAX */

#include <stdio.h>  /* For fprintf() */
#include <signal.h> /* For signal() to ignore SIGPIPE */
#include <unistd.h> /* For getopt() on some systems */
#include <stdlib.h> /* For getopt() on others */
#include <string.h> /* For strchr(), strtok(), strdup() */
#include <errno.h>  /* For errno */

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <constants.h>  /* Server constants */

#include <string.h>

#include <sweetuam.h>
#include <uamclt.h>
#include <uammsg.h>

#include "../stats_c.h"

#define SWEETGW_VERSION  "v0.4"

#ifndef C_VND_ANY
#define C_VND_ANY  0
#endif

/*
 * TYPES
 */

/* 
 * GLOBALS
 */

/* Command line options */

static int debug;
int global_debug_flag; /* XXX */
static const char *stats_file;
static int stats_interval_min = 5;
static int no_external;
static const int recv_timeout_retries = 3;

/*
 * FUNCTIONS
 */

static void usage() {
    fprintf(stderr, "Usage: sweetgw [-h?] [-v] [-d] [-n] [-c config-file] [-s statfile] [-i minutes]\n");
    _exit(1);
}

#ifdef __STDC__
static void msg(const char *format, ...) {
#else
static void msg(va_alist) va_dcl {
#endif
#ifndef __STDC__
  char *format;
#endif
  va_list pvar;
  static char b[1024];
  time_t now = time(NULL);

#ifdef __STDC__
  va_start(pvar, format);
#else
  va_start(pvar);
  format = va_arg(pvar, char *);
#endif

  strcpy(b, asctime(localtime(&now)));
  b[strlen(b)-1] = ' ';

  vsnprintf(b+strlen(b), sizeof(b)-strlen(b), format, pvar);
  strncat(b, "\n", sizeof(b)-1);
  b[sizeof(b)-1] = '\0';
  va_end(pvar);

  if (write(2, b, strlen(b)) == -1)
    exit(-1);
}

static uint32_t set_acknak(uint32_t *msgbuf, int acknack) {
    static const char *func = "get_val_by_name():";
    if (debug) {
      msg("%s %s reported", func, acknack >= 0 ? "ACK" : "NAK");
    }
    msgbuf[0] = htonl(0xdeadbeef);
  	msgbuf[1] = htonl(28);
	msgbuf[2] = htonl(C_DS_INTERNAL); msgbuf[3] = htonl(C_VND_ANY);
	msgbuf[4] = htonl(C_DI_INT); msgbuf[5] = htonl(4);
	msgbuf[6] = acknack; /* XXX error recovery protocol */
	return 7 << 2;
}

static int format_uam_request(char *dst, size_t dstsz, char **argv) {
    size_t used = 0;
    int i;

    if (dstsz == 0) {
      return 0;
    }

    dst[0] = '\0';
    for (i = 0; argv[i] != NULL; i++) {
      int written = snprintf(dst + used, dstsz - used,
                             "%s\"%s\"",
                             i == 0 ? "" : " ",
                             argv[i]);
      if (written < 0 || (size_t)written >= dstsz - used) {
        used = dstsz - 1;
        break;
      }
      used += (size_t)written;
    }
    dst[used] = '\0';
    return (int)used;
}

int main(int argc, char **argv) {
    static uint32_t msgbuf[(C_MAX_MSGSIZE >> 2) + 1];
    static char func[32];
    sw_uamclt_group_t *group;
    uint32_t spc, vnd, atr, len, *i, *e, *o;
    char c;
    int avpair_idx;
    int n, m, idx, sw_argc;
    char *sw_argv[SW_UAM_ARG_MAX];
    /*
    ** Individual args may be quite large, the only limit is UAM message 
    ** size, which is checked elsewhere. Therefore we practically put no 
    ** limit on individual arg size.
    */
    char argb[SW_UAM_ARG_MAX][SW_UAM_MSG_SIZE];
    const char *config_filename = sw_uamclt_default_config_file;

    sprintf(func, "sweetgw[%d]:", getpid());

    /*
    ** Parse options 
    */

    /* Handle options */
    while((c = getopt(argc, argv, "?hvdc:s:i:n")) != -1) {
        switch(c) {
        case 'd': 
          debug++; 
          global_debug_flag = 5;
          break;
        case 'n':
          no_external = 1;
          break;
        case 's':
          stats_file = optarg;
          break;
        case 'i':
          stats_interval_min = atoi(optarg);
          break;
        case 'c':
          config_filename = optarg;
          break;
        case 'v':
          fprintf(stderr, "\nsweetgw module " SWEETGW_VERSION ". "
                  "Copyright (C) 2006 Golden Telecom (R).\n\n"
                  "Permission to redistribute an original or modified version of this program\n"
                  "in source, intermediate or object code form is hereby granted exclusively\n"
                  "under the terms of the GNU General Public License, version 2. Please see the\n"
                  "file COPYING for details, or refer to http://www.gnu.org/copyleft/gpl.html.\n");
        case 'h':
        case '?':
          usage();
        }
    }

    /* Handle positional argument */
    if (optind != argc) { 
        fprintf(stderr, "%s unexpected command-line params\n", func);
        usage(); 
    }

    if (debug) msg("%s debugging enabled", func);

    if (sw_config_load(config_filename) == -1)
      exit(-1);

    if (sw_uam_create_client(&group) == -1)
      exit(-1);

    stats_c_t stats;
    stats_c_init(&stats, "sweetgw", stats_file, stats_interval_min);

    /* Request loop */
    if (debug) msg("%s ready", func);

    for(;;) {
        uint64_t stats_start_ms = stats_c_now_ms();
        int stats_pending = 1;
        /*
         * Get the request from OpenRADIUS
         */

        if (debug) msg("%s fell asleep", func);

        /* Read header */
        if (read(0, msgbuf, 8) != 8) { 
          perror("sweetgw: read"); 
          break; 
        }

        if (debug) msg("%s *** new msg", func);

        if (ntohl(msgbuf[0]) != 0xbeefdead && ntohl(msgbuf[0]) != 0xdeadbeef) {
            msg("%s invalid magic 0x%08x!", func, ntohl(msgbuf[0])); 
            break;
        }
        len = ntohl(msgbuf[1]);
        if (len < 8 || len > sizeof(msgbuf) - 4) {
            msg("%s invalid length %d!", func, len); 
            break;
        }

        /* Read rest of message */
        if (read(0, msgbuf + 2, len - 8) != len - 8) {
            perror("sweetgw: read"); 
            break;
        }

        if (debug)
            msg("%s %d-octet msg recved from radius core", func, len-6);

        e = msgbuf + (len >> 2);
        sw_argc = idx = 0;
        for(i = msgbuf + 2; i < e; i += ((len + 3) >> 2) + 4) {
            /* Get space/vendor/attribute/length */
            spc = ntohl(i[0]); vnd = ntohl(i[1]);
            atr = ntohl(i[2]); len = ntohl(i[3]);
            if (debug) {
                msg("%s got space %d, vendor %d, attribute %d, len %d", func, spc, vnd, atr, len);
            }
            /* Ignore anything other than str/int */
            if (spc != C_DS_INTERNAL || vnd != C_VND_ANY) {
              if (debug) {
                msg("%s ignoring space %d, vendor %d, attribute %d, len %d", func, spc, vnd, atr, len);
              }
              continue;
            }
            switch(atr) {
            case C_DI_STR:
              if (len >= sizeof(argb[idx])) {
                msg("%s -> str attr #%d too large", func, sw_argc);
                exit(-1);
              }
              memset(argb[idx], 0, sizeof(argb[idx]));
              memcpy(argb[idx], (char *)(i+4), len);
              sw_argv[sw_argc] = argb[idx++];
              if (debug) {
                msg("%s -> UAM arg #%d: \"%s\"", func, sw_argc, sw_argv[sw_argc]);
              }
              sw_argc++;
              break;
            default:
              msg("%s unexpected atr %d", func, atr);
              exit(-1);
            }
        }

        sw_argv[sw_argc++] = NULL;

        if (debug)
          msg("%s built %d-element UAM vector", func, sw_argc);

        if (no_external) {
          o = msgbuf + set_acknak(msgbuf, 1) / 4; /* ACK */
          for(sw_argc=0;sw_argv[sw_argc]; sw_argc++) {
            m = strlen(sw_argv[sw_argc]);
            if (((o - msgbuf + 4 + ((m + 3) >> 2)) << 2) >= C_MAX_MSGSIZE) {
              msg("%s no more room for attribute %d", func, avpair_idx);
              exit(-1);
            }
            memcpy(&(o[4]), sw_argv[sw_argc], m);
            o[0] = htonl(C_DS_INTERNAL);
            o[1] = htonl(C_VND_ANY);
            o[2] = htonl(C_DI_STR);
            o[3] = htonl(m);
            o += 4 + ((m + 3) >> 2);
          }
          m = (o - msgbuf) << 2;
          msgbuf[1] = htonl(m);
          if (write(1, msgbuf, m) != m) { 
            perror("sweetgw: write");
            exit(-1);
          }
          if (stats_pending) {
            stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
            stats_pending = 0;
          }
          continue;
        }

        {
          int recv_attempt;
          int recv_ok = 0;
          int timed_out = 0;
          char request_dump[2048];

          request_dump[0] = '\0';
          format_uam_request(request_dump, sizeof(request_dump), sw_argv);

          for (recv_attempt = 1; recv_attempt <= recv_timeout_retries; recv_attempt++) {
            if (sw_uam_send_msg(group, sw_argv) == -1) {
              len = set_acknak(msgbuf, -1);
              if (write(1, msgbuf, len) != len) {
                perror("sweetgw: write");
                exit(-1);
              }
              if (stats_pending) {
                stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
                stats_pending = 0;
              }
              goto next_request;
            }

            if (debug)
              msg("%s UAM query sent (attempt %d/%d)", func, recv_attempt, recv_timeout_retries);

            errno = 0;
            if (sw_uam_recv_msg(group, sw_argv) == 0) {
              recv_ok = 1;
              break;
            }

            if (errno == 0) {
              timed_out = 1;
              if (recv_attempt < recv_timeout_retries) {
                msg("%s UAM response timeout on attempt %d/%d, retrying", func, recv_attempt, recv_timeout_retries);
                continue;
              }
              msg("%s UAM response timeout after %d attempts, sending: %s", func, recv_timeout_retries, request_dump);
            }
            break;
          }

          if (!recv_ok) {
            if (!timed_out && debug) {
              msg("%s UAM response receive failed without timeout", func);
            }
            len = set_acknak(msgbuf, -1);
            if (write(1, msgbuf, len) != len) {
              perror("sweetgw: write");
              exit(-1);
            }
            if (stats_pending) {
              stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
              stats_pending = 0;
            }
            continue;
          }
        }

        if (debug)
          msg("%s UAM response received", func);

        if (sw_argv[0] == NULL || sw_argv[0][0] != 'O') {
          msg("%s NAK received", func);
          len = set_acknak(msgbuf, -1);
          if (write(1, msgbuf, len) != len) {
            perror("sweetgw: write");
            exit(-1);
          }
          if (stats_pending) {
            stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
            stats_pending = 0;
          }
          continue;
        }

        if (sw_argv[1] == NULL || strcmp(sw_argv[1], argb[1])) {
          msg("%s lost sync %s vs %s", func, sw_argv[1] ? sw_argv[1] : "<empty>", argb[1]);
          len = set_acknak(msgbuf, -1);
          if (write(1, msgbuf, len) != len) {
            perror("sweetgw: write");
            exit(-1);
          }
          if (stats_pending) {
            stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
            stats_pending = 0;
          }
          exit(-1);
        }

        o = msgbuf + set_acknak(msgbuf, 1) / 4; /* ACK */

        for(sw_argc=0;sw_argv[sw_argc]; sw_argc++) {
          /* Check if there's room, considering current and new size */
          m = strlen(sw_argv[sw_argc]);
          if (((o - msgbuf + 4 + ((m + 3) >> 2)) << 2) >= C_MAX_MSGSIZE) {
            msg("%s no more room for attribute %d", func, avpair_idx);
            exit(-1);
          }
          memcpy(&(o[4]), sw_argv[sw_argc], strlen(sw_argv[sw_argc]));

          if (debug) 
            msg("%s <- UAM arg #%d: \"%s\"", func, sw_argc, sw_argv[sw_argc]);

          /* Write attribute to message buffer and advance ptr */
          o[0] = htonl(C_DS_INTERNAL);
          o[1] = htonl(C_VND_ANY);
          o[2] = htonl(C_DI_STR);
          o[3] = htonl(m);
          o += 4 + ((m + 3) >> 2);
        }

        m = (o - msgbuf) << 2;
        msgbuf[1] = htonl(m);

        if (debug)
          msg("%s %d UAM elements formatted", func, sw_argc);

        if (write(1, msgbuf, m) != m) { 
          perror("sweetgw: write");
          exit(-1);
        }
        if (stats_pending) {
          stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
          stats_pending = 0;
        }

        if (debug)
          msg("%s %d-octets sent to radius core", func, m);
next_request:
        ;
    }

    sw_uam_destroy_client(group);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, SIG_DFL);

    return 1;
}
