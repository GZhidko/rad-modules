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

#include <stdio.h>  /* For fprintf() */
#include <signal.h> /* For signal() to ignore SIGPIPE */
#include <unistd.h> /* For getopt() on some systems */
#include <stdlib.h> /* For getopt() on others */
#include <string.h> /* For strchr(), strtok(), strdup() */
#include <errno.h>  /* For errno */

#include <constants.h>  /* Server constants */

#include <platform.h>   /* For U_INT32_T */

#include <uamclt.h>

#define SWEETGW_VERSION  "v0.1"

/*
 * TYPES
 */

/* 
 * GLOBALS
 */

/* Command line options */

static int debug;

/*
 * FUNCTIONS
 */

static void usage() {
    fprintf(stderr, "Usage: sweetgw [-h?] [-v] [-d]\n");
    _exit(1);
}

static U_INT32_T set_acknak(U_INT32_T *msgbuf, int acknack) {
    static const char *func = "get_val_by_name():";
    if (debug) {
      fprintf(stderr, "%s %s reported\n", func, acknack ? "ACK" : "NAK");
    }
  	msgbuf[1] = htonl(28);
	msgbuf[2] = htonl(C_DS_INTERNAL); msgbuf[3] = 0;
	msgbuf[4] = htonl(C_DI_INT); msgbuf[5] = htonl(4);
	msgbuf[6] = acknack; /* XXX error recovery protocol */
	return 7 << 2;
}

int main(int argc, char **argv) {
    static U_INT32_T msgbuf[(C_MAX_MSGSIZE >> 2) + 1];
    static char func[32];
    U_INT32_T spc, vnd, atr, len, *i, *e, *o;
    char c;
    int avpair_idx;
    int n, m;

    sprintf(func, "sweetgw[%d]{?}:", getpid());

    /*
    ** Parse options 
    */

    /* Handle options */
    while((c = getopt(argc, argv, "?hvd")) != -1) {
        switch(c) {
        case 'd': 
          debug++; 
          break;
        case 'v':
          fprintf(stderr, "\nsweetgw module " SWEETGW_VERSION ". "
                  "Copyright (C) 2006 Golden Telecom (R).\n\n"
                  "Permission to redistribute an original or modified version of this program\n"
                  "in source, intermediate or object code form is hereby granted exclusively\n"
                  "under the terms of the GNU General Public License, version 2. Please see the\n"
                  "file COPYING for details, or refer to http://www.gnu.org/copyleft/gpl.html.\n\n");
        case 'h':
        case '?':
          usage();
        }
    }

    /* Handle positional argument */
    if (optind != argc) { 
        fprintf(stderr, "%s unexpected command-line params\n\n", func);
        usage(); 
    }

    if (debug) fprintf(stderr, "%s debugging enabled\n", func);

    if (sw_config_load(config_filename) == -1)
      exit(-1);

    uam_client_fd = sw_uam_create_client();
    if (uam_client_fd == -1)
      exit(-1);

    /* Request loop */
    if (debug) fprintf(stderr, "%s ready\n", func);

    for(;;) {
        /*
         * Get the request from OpenRADIUS
         */

        if (debug) fprintf(stderr, "%s fell asleep\n", func);

        /* Read header */
        if (read(0, msgbuf, 8) != 8) { perror("sweetgw: read"); break; }

        if (debug) fprintf(stderr, "%s *** new msg\n", func);

        if (ntohl(msgbuf[0]) != 0xdeadbeef) {
            fprintf(stderr, "%s invalid magic 0x%08x!\n", 
                    func, ntohl(msgbuf[0])); 
            break;
        }
        len = ntohl(msgbuf[1]);
        if (len < 8 || len > sizeof(msgbuf) - 4) {
            fprintf(stderr, "%s invalid length %d!\n", func, len); 
            break;
        }

        /* Read rest of message */
        if (read(0, msgbuf + 2, len - 8) != len - 8) {
            perror("sweetgw: read"); 
            break;
        }

        if (debug)
            fprintf(stderr, "%s %d-octet msg recved from radius core\n",
                    func, len-6);

        e = msgbuf + (len >> 2);
        sw_argc = 0;
        for(i = msgbuf + 2; i < e; i += ((len + 3) >> 2) + 4) {
            /* Get space/vendor/attribute/length */
            spc = ntohl(i[0]); vnd = ntohl(i[1]);
            atr = ntohl(i[2]); len = ntohl(i[3]);
            if (debug) {
                fprintf(stderr, "%s got space %d, vendor %d, attribute %d, len %d\n", func, spc, vnd, atr, len);
            }
            /* Ignore anything other than str/int */
            if (spc != C_DS_INTERNAL || vnd != 0) {
              if (debug) {
                fprintf(stderr, "%s ignoring space %d, vendor %d, attribute %d, len %d\\n", func, spc, vnd, atr, len);
              }
              continue;
            }
            switch(atr) {
            case C_DI_STR:
              sw_argv[sw_argc++] = (char *)(i+4);
              if (debug) {
                char t[4096];
                memset(t, 0, sizeof(t));
                memcpy(t, (char *)(i+4), len < sizeof(t) ? len : sizeof(t)-1);
                fprintf(stderr, "%s -> str val \"%s\"\n", func, t);
              }
              break;

            default:
              fprintf(stderr, "%s unexpected atr %d\n", func, atr);
              exit(-1);
            }
        }

        sw_argv[argc++] = NULL;

        if (sw_uam_send_msg(uam_client_fd, sw_argv) == -1)
          exit(-1);

        if (debug)
          fprintf(stderr, "%s msg sent to Sweetspot\n", func);

        if (sw_uam_recv_msg(uam_client_fd, sw_argv) == -1)
          exit(-1);

        if (debug)
          fprintf(stderr, "%s %d-octet msg recved from Sweetspot\n", func, rc);

        for(sw_argc=0;sw_argv[sw_argc]; sw_argc++) {
          /* Check if there's room, considering current and new size */
          m = strlen(sw_argv[sw_argc]);
          if (((o - msgbuf + 4 + ((m + 3) >> 2)) << 2) >= C_MAX_MSGSIZE) {
            fprintf(stderr, "%s no more room for attribute %d\n",
                    func, avpair_idx);
            exit(-1);
          }

          memcpy(&(o[4]), sw_argv[sw_argc], strlen(sw_argv[sw_argc]));
          if (debug) fprintf(stderr, "%s <- str val \"%s\"\n", 
                             func, sw_argv[sw_argc]);

          /* Write attribute to message buffer and advance ptr */
          o[0] = htonl(C_DS_INTERNAL);
          o[1] = htonl(0);
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

        if (debug)
          fprintf(stderr, "%s %d-octet msg sent to radius core\n", func, m);
    }

    sw_uam_destroy_client(uam_client_fd);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, SIG_DFL);

    return 1;
}
