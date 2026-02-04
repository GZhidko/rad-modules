/*
** absgw -- OpenRADIUS <--> ABS gateway module
**
** Sends request to arbitrary ABS service along with a configured set
** of RADIUS attributes, optionally receives a response, then builds
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
#include <sysexits.h>   /* For values to indicate a condition when ending a program */

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <constants.h>  /* Server constants */

#include <platform.h>   /* For U_INT32_T */

#include <abs/msg.h>
#include <abs/io.h>
#include <abs/v3.h>

#include "../stats_c.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
** Hack alert:
**
** OR poke SPC, VND, and other into 32bit chanks.
** C_VND_ANY represents a special number 0x800...000.
** On 64bit platforms OR cat out 0x80...00 part, and
** we get only 0x00...00 (zero).
**
** This hack do not work if SIZEOF_SIGNED_LONG < 4.   
*/
#if SIZEOF_SIGNED_LONG == 4
#ifndef C_VND_ANY
#define C_VND_ANY ((U_INT32_T)0x80000000)
#endif
#else
#define C_VND_ANY ((U_INT32_T)0x80000000)
#endif

/*
 * TYPES
 */

/* 
 * GLOBALS
 */

/* Command line options */

static int debug;
static const char *stats_file;
static int stats_interval_min = 5;
static int no_external;

/*
 * FUNCTIONS
 */

static void usage() {
    fprintf(stderr, "Usage: " PACKAGE " [-h?] [-v] [-V] [-d] [-n] [-t ttl] [-s statfile] [-I minutes] <-c abs-service-id > < -i input-attr-map > < -o output-attr-map >\n");
    exit(EX_USAGE);
}

#ifdef __STDC__
static void logit(const char *format, ...) {
#else
static void logit(va_alist) va_dcl {
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
    exit(EX_IOERR);
}

static U_INT32_T set_acknak(U_INT32_T *msgbuf, int acknack) {
    static const char *func = "set_acknak():";
    if (debug) {
      logit("%s %s reported", func, acknack >= 0 ? "ACK" : "NAK");
    }
    msgbuf[0] = htonl(0xdeadbeef);
  	msgbuf[1] = htonl(28);
	msgbuf[2] = htonl(C_DS_INTERNAL); msgbuf[3] = htonl(C_VND_ANY);
	msgbuf[4] = htonl(C_DI_INT); msgbuf[5] = htonl(4);
	msgbuf[6] = acknack; /* XXX error recovery protocol */
	return 7 << 2;
}

/* Walk down [possible] sequence of subnames, return value for first match */
static int get_val_by_name(abs_v3_tbl_t *tbl, char **subnames, 
                           abs_v3_ent_type_t *type, 
                           void **value, size_t *lvalue) {
  abs_v3_tbl_t *sub_tbl;
  int col, subname_idx, rc;
  char *attr;
  static const char *func = "get_val_by_name():";

  if (subnames == NULL || subnames[0] == NULL) {
    return -1;
  }

  subname_idx = 0;

  for(col=0; subnames[subname_idx]; col++) {
    if ((rc = abs_v3_tbl_get(tbl, 0, col, NULL, 
                             (void **)&attr, NULL)) < 0) {
      logit("%s abs_v3_tbl_get() failed at 0:%d for \"%s\": %s", func, col, subnames[subname_idx], abs_v3_error_str(rc));
      return -1;
    }
    if (strcmp(attr, subnames[subname_idx])) {
      if (debug) {
        logit("%s %s != %s", func, attr, subnames[subname_idx]);
      }
      continue;
    }
    if (debug) {
      logit("%s %s == %s", func, attr, subnames[subname_idx]);
    }
    subname_idx++;
    if (subnames[subname_idx] == NULL) { /* Leaf */
      if ((rc = abs_v3_tbl_get(tbl, 1, col, type, value, lvalue)) < 0) {
        logit("%s abs_v3_tbl_get() failed at 1:%d for \"%s\": %s", func, col, attr, abs_v3_error_str(rc));
        return -1;
      }
      return 0;
    } else { /* Branch */
      if ((rc = abs_v3_tbl_get(tbl, 1, col, type, 
                               (void *)&sub_tbl, NULL)) < 0) {
        logit("%s abs_v3_tbl_get() failed at 1:%d for \"%s\": %s", func, col, attr, abs_v3_error_str(rc));
        return -1;
      }
      if (*type != ABS_V3_ENT_TYPE_TABLE) {
        logit("%s non table-type subtable at 1:%d for \"%s\"", func, col, attr);
        return -1;
      }
      return get_val_by_name(sub_tbl, &subnames[subname_idx], type, value, lvalue);
    }
  }
  if (debug) logit("%s unmatched var %s...", func, subnames[0]);
  return -1;
}

static int set_val_by_name(abs_v3_tbl_t **tbl, char **subnames, 
                           abs_v3_ent_type_t type,
                           const void *value, const size_t lvalue) {
  abs_v3_tbl_t *sub_tbl;
  abs_v3_ent_type_t existing_type;
  int subname_idx, col, rc;
  char *attr;
  static const char *func = "set_val_by_name()";

  if (*tbl == NULL) {
    if ((rc = abs_v3_tbl_create(tbl)) < 0) {
      logit("%s abs_v3_tbl_create() failed: %s", func, abs_v3_error_str(rc));
      exit(EX_DATAERR);
    }
  }

  if (subnames == NULL || subnames[0] == NULL) {
    return -1;
  }

  subname_idx = 0;

  for(col=0; subnames[subname_idx]; col++) {
    rc = abs_v3_tbl_get(*tbl, 0, col, NULL, 
                        (void **)&attr, NULL);
    if (rc < 0) {
      if (rc == ABS_V3_ERR_OUTOFCOL || rc == ABS_V3_ERR_OUTOFROW) {
        /* No such entry yet -- create one */
        if ((rc = abs_v3_tbl_set(*tbl, 0, col, ABS_V3_ENT_TYPE_STRING, 
                                 subnames[subname_idx], -1)) < 0) {
          logit("%s abs_v3_tbl_set() failed at 0:%d for \"%s\": %s", func, col, subnames[subname_idx], abs_v3_error_str(rc));
          return -1;
        }
        attr = subnames[subname_idx];
      } else {
        logit("%s abs_v3_tbl_get() failed at 0:%d for \"%s\": %s", func, col, subnames[subname_idx], abs_v3_error_str(rc));
        return -1;
      }
    }
    if (strcmp(attr, subnames[subname_idx])) {
      if (debug) {
        logit("%s %s != %s", func, attr, subnames[subname_idx]);
      }
      continue;
    }
    if (debug) {
      logit("%s %s == %s", func, attr, subnames[subname_idx]);
    }
    rc = abs_v3_tbl_get(*tbl, 1, col, &existing_type, (void **)&sub_tbl, NULL);
    if (rc < 0) {
      sub_tbl = NULL;
    } else {
      switch (existing_type) {
      case ABS_V3_ENT_TYPE_TABLE:
        break;
      case ABS_V3_ENT_TYPE_EMPTY:
        sub_tbl = NULL;
        break;
      default:
        logit("%s cell overwrite denied at %s", func, subnames[subname_idx]);
        return -1;
      }
    }
    subname_idx++;
    if (subnames[subname_idx] == NULL) { /* Leaf */
      if ((rc = abs_v3_tbl_set(*tbl, 1, col, type, (void *)value, \
                               lvalue)) < 0) {
        logit("%s abs_v3_tbl_get() failed at 1:%d for \"%s\": %s", func, col, attr, abs_v3_error_str(rc));
        return -1;
      }
      return 0;
    } else { /* Branch */
      if (set_val_by_name(&sub_tbl, &subnames[subname_idx], \
                          type, value, lvalue) == -1) {
        logit("%s sub-set_val_by_name() failed", func);
        return -1;
      }
      if ((rc = abs_v3_tbl_set(*tbl, 1, col, ABS_V3_ENT_TYPE_TABLE, 
                               sub_tbl, -1)) < 0) {
        logit("%s abs_v3_tbl_set() failed at 1:%d for \"%s\": %s", func, col, attr, abs_v3_error_str(rc));
        return -1;
      }
      return 0;
    }
  }
  /* XXX format subnames */
  if (debug) logit("%s unmatched var %s...", func, subnames[0]);
  return -1;
}

int main(int argc, char **argv) {
    static U_INT32_T msgbuf[(C_MAX_MSGSIZE >> 2) + 1];
    static char func[32];
    U_INT32_T spc, vnd, atr, len, *i, *e, *o;
    char c, **subnames, **input_attrs[64], **output_attrs[64]; /* check XXX */
    int rc, col, abs_service_id=0, abs_oneway_flag=0, abs_rsp_time=0;
    int req_serial = 0, rsp_serial, errcode;
    int avpair_idx;
    int n, m;
    void *p1, *p2;
    abs_io_t *io;
    abs_msg_t *msg;
    abs_v3_tbl_t *tbl;
    abs_v3_ent_type_t type;
    void *value;
    size_t lvalue;
    u_char *pdu, *data, ibuf[65536];
    size_t lpdu, ldata, iused = 0;

    sprintf(func, PACKAGE "[%d]{?}:", getpid());

    input_attrs[0] = output_attrs[0] = NULL;

    /*
    ** Parse options 
    */

    /* Handle options */
    while((c = getopt(argc, argv, "?hvVdn1t:c:i:o:s:I:")) != -1) {
        switch(c) {
        case 's':
          stats_file = optarg;
          break;
        case 'I':
          stats_interval_min = atoi(optarg);
          break;
        case 'n':
          no_external = 1;
          break;
        case '1':
          abs_oneway_flag = 1;
          break;
        case 't':
          abs_rsp_time = atoi(optarg);
          break;
        case 'c':
          abs_service_id = atoi(optarg);
          sprintf(func, PACKAGE "[%d]{%d}:", getpid(), abs_service_id);
          break;
        case 'i':
        case 'o': {
          char subname[64];
          char *b = optarg, *p = optarg;
          int subname_idx;

          avpair_idx = subname_idx = 0;

          if (c == 'i') {
            input_attrs[avpair_idx] = malloc(sizeof(char *)*64);
            input_attrs[avpair_idx][subname_idx] = NULL;
          } else {
            output_attrs[avpair_idx] = malloc(sizeof(char *)*64);
            output_attrs[avpair_idx][subname_idx] = NULL;
          }

          while (1) {
            if (*p == ',') {
              if (c == 'i') {
                input_attrs[avpair_idx+1] = malloc(sizeof(char *)*64);
                input_attrs[avpair_idx+1][subname_idx] = NULL;
              } else {
                output_attrs[avpair_idx+1] = malloc(sizeof(char *)*64);
                output_attrs[avpair_idx+1][subname_idx] = NULL;
              }
            }
            if (*p == '\0' || *p == '.' || *p == ',') {
              memset(subname, 0, sizeof(subname));
              strncpy(subname, b, p-b);
              if (c == 'i') {
                input_attrs[avpair_idx][subname_idx] = strdup(subname);
              } else {
                output_attrs[avpair_idx][subname_idx] = strdup(subname);
              }
              subname_idx++;
              b = p+1;
            }
            if (*p == '\0' || *p == ',') {
              avpair_idx++; subname_idx = 0;  /* Next name */
            }
            if (*p == '\0') {
              if (c == 'i') {
                input_attrs[avpair_idx] = NULL;
              } else {
                output_attrs[avpair_idx] = NULL;
              }
              break;
            }
            if (*p == ',') {
              b = p+1;
            }
            p++;
          }
          break;
        }
        case 'd': 
          debug++; 
          break;
        case 'V':
          fprintf(stderr, "\n" PACKAGE " " VERSION "\n"
	          "Compile-time options:\n"
		  "  SIZEOF_SIGNED_LONG = %lx\n"
		  "  C_VND_ANY = %lx\n",
		  SIZEOF_SIGNED_LONG, C_VND_ANY);
          exit(EXIT_SUCCESS);
        case 'v':
          fprintf(stderr, "\n" PACKAGE " module " VERSION ". "
                  "Copyright (C) 2004 Golden Telecom (R).\n\n"
                  "Permission to redistribute an original or modified version of this program\n"
                  "in source, intermediate or object code form is hereby granted exclusively\n"
                  "under the terms of the GNU General Public License, version 2. Please see the\n"
                  "file COPYING for details, or refer to http://www.gnu.org/copyleft/gpl.html.\n");
          exit(EXIT_SUCCESS);
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

    if (!abs_service_id) {
      fprintf(stderr, "%s unfortunate ABS service ID\n", func);
      usage();
    }

    if (debug) logit("%s debugging enabled", func);

    /*
    ** Setup ABS connection
    */

    /* Create io session to local middleware server */
    if (abs_io_meta_create(&io, NULL, NULL) < 0) {
      const char *ptr;
      while ((ptr = abs_io_log_pop(io, NULL)) != NULL)
        logit("%s %s", func, ptr);
      exit(EX_UNAVAILABLE);
    }

    /* Set custom I/O response timeout */
    if (abs_rsp_time && \
        abs_io_cfg_seti(io, ABS_IO_KEY_TIMEOUT, abs_rsp_time+1) < 0)
    {
      const char *ptr;
      while ((ptr = abs_io_log_pop(io, NULL)) != NULL)
        logit("%s %s", func, ptr);
      exit(EX_PROTOCOL);
    }

    /* Set blocking I/O */
    if (abs_io_cfg_seti(io, ABS_IO_KEY_BLOCKING, 1) < 0)
    {
      const char *ptr;
      while ((ptr = abs_io_log_pop(io, NULL)) != NULL)
        logit("%s %s", func, ptr);
      exit(EX_PROTOCOL);
    }

    stats_c_t stats;
    stats_c_init(&stats, "absgw", stats_file, stats_interval_min);

    /* Request loop */
    if (debug) logit("%s ready", func);

    for(;;) {
        uint64_t stats_start_ms = stats_c_now_ms();
        int stats_pending = 1;
        /*
         * Get the request from OpenRADIUS
         */

        if (debug) logit("%s fell asleep", func);

        /* Read header */
        if (read(0, msgbuf, 8) != 8) { perror(PACKAGE ": read"); break; }

        if (debug) logit("%s *** new msg", func);

        if (ntohl(msgbuf[0]) != 0xbeefdead && ntohl(msgbuf[0]) != 0xdeadbeef) {
            logit("%s invalid magic 0x%08x!", func, ntohl(msgbuf[0])); 
            break;
        }
        len = ntohl(msgbuf[1]);
        if (len < 8 || len > sizeof(msgbuf) - 4) {
            logit("%s invalid length %d!", func, len); 
            break;
        }
 
        /* Read rest of message */
        if (read(0, msgbuf + 2, len - 8) != len - 8) {
            perror(PACKAGE ": read"); 
            break;
        }

        if (debug)
          logit("%s %d-octet msg recved from radius core", func, len-6);

        if (no_external) {
          if (write(1, msgbuf, len) != len) {
            perror(PACKAGE ": write");
            exit(EX_IOERR);
          }
          if (stats_pending) {
            stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
            stats_pending = 0;
          }
          continue;
        }

        e = msgbuf + (len >> 2);
        col = 0; avpair_idx = 0;
        tbl = NULL;
        for(i = msgbuf + 2; i < e; i += ((len + 3) >> 2) + 4) {
            /* Get space/vendor/attribute/length */
            spc = ntohl(i[0]); vnd = ntohl(i[1]);
            atr = ntohl(i[2]); len = ntohl(i[3]);
            if (debug) {
                logit("%s got space %d, vendor %d, attribute %d, len %d", func, spc, vnd, atr, len);
            }
            /* Ignore anything other than str/int */
            if (spc != C_DS_INTERNAL || vnd != C_VND_ANY) {
              if (debug) {
                logit("%s ignoring space %d, vendor %d, attribute %d, len %d", func, spc, vnd, atr, len);
              }
              continue;
            }
            /* Get attr name */
            subnames = input_attrs[avpair_idx++];
            if (subnames == NULL) {
              logit("%s unbound attr on input", func);
              exit(EX_DATAERR);
            }
            if (debug) {
              int idx;
              logit("%s -> attr ", func);
              for(idx=0; subnames[idx]; idx++) {
                logit("%s", subnames[idx]);
              }
              logit("");
            }
            switch(atr) {
            case C_DI_INT:
              memcpy(&m, (char *)(i+4), len);
              m = htonl(m);
              rc = set_val_by_name(&tbl, subnames, ABS_V3_ENT_TYPE_INTEGER,
                                   &m, sizeof(m));
              if (debug) logit("%s -> int val %d", func, m);
              break;

            case C_DI_STR:
              rc = set_val_by_name(&tbl, subnames, ABS_V3_ENT_TYPE_STRING,
                                  (char *)(i+4), len);
              if (debug) {
                char t[4096];
                memset(t, 0, sizeof(t));
                memcpy(t, (char *)(i+4), len < sizeof(t) ? len : sizeof(t)-1);
                logit("%s -> str val \"%s\"", func, t);
              }
              break;

            default:
              logit("%s unexpected atr %d", func, atr);
              exit(EX_DATAERR);
            }
            if (rc == -1) {
              logit("%s set_val_by_name() failed", func);
              exit(EX_DATAERR);
            }
            col++;
        }

        if (input_attrs[avpair_idx] != NULL) {
          logit("%s unused attr on input", func);
          exit(EX_DATAERR);
        }

        pdu = NULL; lpdu = 0;

        if ((rc = abs_v3_tbl_marshal(&pdu, &lpdu, tbl)) < 0) {
            logit("%s abs_v3_tbl_marshal() failed: %s", func, abs_v3_error_str(rc));
            exit(EX_DATAERR);
        }
    
        abs_v3_tbl_destroy(tbl);

        if (req_serial > 0xffffff)
          req_serial = 0;
        else
          req_serial++;

        if ((rc = abs_msg_create(&msg) < 0)) {
          logit("%s abs_msg_create() failed: %s", func, abs_msg_error_str(rc));
          free(pdu);
          exit(EX_DATAERR);
        }

        if ((rc = abs_msg_set(msg, ABS_MSG_ITEM_SERIAL,
                              &req_serial, sizeof(req_serial))) < 0 ||
            (rc = abs_msg_set(msg, ABS_MSG_ITEM_SERVICE,
                              &abs_service_id, sizeof(abs_service_id))) < 0 ||
            (rc = abs_msg_set(msg, ABS_MSG_ITEM_PROPERTIES, \
                              &abs_oneway_flag, sizeof(abs_oneway_flag)))<0 ||
            (rc = abs_msg_set(msg, ABS_MSG_ITEM_TTL, \
                              &abs_rsp_time, sizeof(abs_rsp_time))) < 0 ||
            (rc = abs_msg_set(msg, ABS_MSG_ITEM_PDU, pdu, lpdu)) < 0) {
          free(pdu);
          abs_msg_destroy(msg);
          logit("%s abs_msg_set() failed: %s", func, abs_msg_error_str(rc));
          exit(EX_DATAERR);
        }

        if ((rc = abs_msg_marshal(&data, &ldata, msg)) < 0) {
          free(pdu);
          abs_msg_destroy(msg);
          logit("%s abs_msg_marshal() failed: %s", func, abs_msg_error_str(rc));
          exit(EX_DATAERR);
        }

        abs_msg_destroy(msg);

        free(pdu);

        /* Send message to server */
        rc = abs_io_tr_send(io, data, ldata);
        if (rc < 0 || rc != ldata) {
            const char *ptr;
            while ((ptr = abs_io_log_pop(io, NULL)) != NULL) {
                logit("%s %s", func, ptr);
            }
            free(data);
            exit(EX_PROTOCOL);
        }

        free(data);

        if (debug)
          logit("%s %d-octet msg sent to ABS", func, rc);

        if (abs_oneway_flag) {
          len = set_acknak(msgbuf, 1);
          if (write(1, msgbuf, len) != len) {
            perror(PACKAGE ": write");
            exit(EX_IOERR);
          }
          if (stats_pending) {
            stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
            stats_pending = 0;
          }
          continue;
        }

        /* Wait for new messages to come from server (blocking) */
        while(1) {
            rc = abs_io_tr_recv(io, ibuf+iused, sizeof(ibuf)-iused);
            if (rc <= 0) {
                const char *ptr;
                while ((ptr = abs_io_log_pop(io, NULL)) != NULL) {
                    logit("%s %s", func, ptr);
                }
                exit(EX_PROTOCOL);
            }
            iused += rc;

            if (debug)
              logit("%s %d-octet msg recved from ABS", func, rc);

            /* Watch for overflow XXX */
            if (iused == sizeof ibuf) {
                logit("%s input overflown", func);
                exit(EX_DATAERR);
            }

            if ((rc = abs_msg_demarshal(&msg, (u_char *)ibuf, iused)) < 0) {
                if (rc == ABS_MSG_ERR_INCOMPLETE) {
                  if (debug)
                    logit("%s incomplete message", func);
                  continue;
                }
                logit("%s abs_msg_demarshal() failure: %s", func, abs_msg_error_str(rc));
                exit(EX_DATAERR);
            }

            if (debug)
                logit("%s %d-octet ABS msg decoded", func, rc);

            /* Strip message off the input */
            iused -= rc;
            memmove(ibuf, ibuf+rc, iused);

            if ((rc = abs_msg_get(msg, ABS_MSG_ITEM_SERIAL,
                                  &p1, NULL)) < 0 ||
                (rc = abs_msg_get(msg, ABS_MSG_ITEM_ERR_CODE,
                                  &p2, NULL)) < 0 ||
                (rc = abs_msg_get(msg, ABS_MSG_ITEM_PDU,
                                  (void **)&pdu, &lpdu)) < 0) {
              abs_msg_destroy(msg);
              logit("%s abs_msg_get() failed: %s", func, abs_msg_error_str(rc));
              exit(EX_DATAERR);
            }

            rsp_serial = *(int *)p1;
            errcode = *(int *)p2;

            if (rsp_serial != req_serial) {
                if (debug)
                    logit("%s mismatched serials %d vs %d", func, req_serial, rsp_serial);
                abs_msg_destroy(msg);
                continue;
            }          
            break;
        }

        len = 0;

        if (errcode != ABS_MSG_NETERR_NOERROR) {
            logit("%s ABS error %s", func, abs_msg_neterr_str(errcode));
            abs_msg_destroy(msg);
            len = set_acknak(msgbuf, -1);
            if (write(1, msgbuf, len) != len) {
                perror(PACKAGE ": write");
                exit(EX_DATAERR);
            }
            continue;
        }

        tbl = NULL;

        if ((rc = abs_v3_tbl_demarshal(&tbl, pdu, lpdu)) < 0) {
            logit("%s abs_v3_tbl_demarshal() failure: %s", func, abs_v3_error_str(rc));
            abs_msg_destroy(msg);
            exit(EX_DATAERR);
        }

        abs_msg_destroy(msg);

        o = msgbuf + set_acknak(msgbuf, 1) / 4; /* ACK */

        avpair_idx = 0;

        while((subnames = output_attrs[avpair_idx++]) != NULL) {
          if (get_val_by_name(tbl, subnames, &type, &value, &lvalue) == -1) {
            logit("%s get_val_by_name() failure", func);
            o = msgbuf + set_acknak(msgbuf, -1) / 4;
            break;
          }

          /* Check if there's room, considering current and new size */
          m = lvalue;
          if (((o - msgbuf + 4 + ((m + 3) >> 2)) << 2) >= C_MAX_MSGSIZE) {
            logit("%s no more room for attribute %d", func, avpair_idx);
            o = msgbuf + set_acknak(msgbuf, -1) / 4;
            break;
          }

          switch(type) {
          case ABS_V3_ENT_TYPE_STRING:
            memcpy(&(o[4]), value, lvalue);
            if (debug) logit("%s <- str val \"%s\"", func, (char *)value);
            break;
          case ABS_V3_ENT_TYPE_INTEGER:
            memcpy(&n, value, sizeof(n));
            o[4] = htonl(n);
            if (debug) logit("%s <- int val %d", func, n);
            break;
          default:
            logit("%s unsupported value type %d in response attr %d", func, type, avpair_idx);
            continue;
          }

          /* Write attribute to message buffer and advance ptr */
          o[0] = htonl(C_DS_INTERNAL);
          o[1] = htonl(C_VND_ANY);
          o[2] = htonl(type == ABS_V3_ENT_TYPE_STRING ? C_DI_STR : C_DI_INT);
          o[3] = htonl(m);
          o += 4 + ((m + 3) >> 2);
        }
        abs_v3_tbl_destroy(tbl);

        m = (o - msgbuf) << 2;
        msgbuf[1] = htonl(m);

        if (write(1, msgbuf, m) != m) { 
          perror(PACKAGE ": write");
          exit(EX_IOERR);
        }
        if (stats_pending) {
          stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
          stats_pending = 0;
        }

        if (debug)
          logit("%s %d-octet msg sent to radius core", func, m);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, SIG_DFL);

    return EX_SOFTWARE;
}
