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

#include <stdio.h>  /* For fprintf() */
#include <signal.h> /* For signal() to ignore SIGPIPE during LDAP unbind */
#include <unistd.h> /* For getopt() on some systems */
#include <stdlib.h> /* For getopt() on others */
#include <string.h> /* For strchr(), strtok(), strdup() */
#include <errno.h>  /* For errno */

#include <constants.h>  /* Server constants */

#include <platform.h>   /* For U_INT32_T */

#include <abs/msg.h>
#include <abs/io.h>
#include <abs/v3.h>

#define RADABS_VERSION  "v0.1"


#ifndef RADABS_MAPFILE
#define RADABS_MAPFILE  "modules/radabs.attrmap"
#endif

#define SEARCH_TIMEOUT  15      /* Search timeout - should be shorter
                       than what's in 'configuration' */

#define MAPF_MAXENTRIES 128     /* Max. entries in mapping file */
#define MAPF_LINELEN    512     /* Max. mapping file line length */
#define LDAP_ATRNAMELEN 256     /* Max. length for LDAP attr. names */

/*
 * TYPES
 */

/* 
 * GLOBALS
 */

struct {
	const char *name;	/* Zero terminated */
	U_INT32_T spcs;
	U_INT32_T vnds;
	U_INT32_T atrs;
    abs_v3_ent_type_t type;
} attrmap[] = {
  /* Proprietary ABS attrs */
  { "str", 115, 0, 1, ABS_V3_ENT_TYPE_STRING },
  { "rc", 115, 0, 2, ABS_V3_ENT_TYPE_INTEGER },
  /* RADIUS<->ABS attr mapping */
  { "service_type", 2, 0, 6, ABS_V3_ENT_TYPE_INTEGER },
  { "framed_protocol", 2, 0, 7, ABS_V3_ENT_TYPE_INTEGER },
  { "framed_ip_address", 2, 0, 8, ABS_V3_ENT_TYPE_INTEGER },
  { "filter_id", 2, 0, 11, ABS_V3_ENT_TYPE_STRING },
  { "login_ip_host", 2, 0, 14, ABS_V3_ENT_TYPE_INTEGER },
  { "login_service", 2, 0, 15, ABS_V3_ENT_TYPE_INTEGER },
  { "login_tcp_port", 2, 0, 16, ABS_V3_ENT_TYPE_INTEGER },
  { "reply_message", 2, 0, 18, ABS_V3_ENT_TYPE_STRING },
  { "session_timeout", 2, 0, 27, ABS_V3_ENT_TYPE_INTEGER },
  { "idle_timeout", 2, 0, 28, ABS_V3_ENT_TYPE_INTEGER },
  { "simultaneous_use", 2, 0, 1024, ABS_V3_ENT_TYPE_INTEGER },
  { NULL, 0, 0, 0, 0 }
};

const char *str_to_abs[] = {
    "uname",
    "realm",
    "rname",
    NULL
};

const char *int_to_abs[] = {
    NULL
};
    
/* Command line options */

static int debug;

/*
 * FUNCTIONS
 */


void usage()
{
    fprintf(stderr, "Usage: radabs [-d] [-v] [-h]\n");
    _exit(1);
}

U_INT32_T set_nak(U_INT32_T *msgbuf)
{
	msgbuf[1] = htonl(28);
	msgbuf[2] = htonl(C_DS_INTERNAL); msgbuf[3] = 0;
	msgbuf[4] = htonl(C_DI_INT); msgbuf[5] = htonl(4);
	msgbuf[6] = 0;

	return 7 << 2;
}

int main(int argc, char **argv)
{
    static U_INT32_T msgbuf[(C_MAX_MSGSIZE >> 2) + 1];
    U_INT32_T spc, vnd, atr, len, *i, *e, *o;
    char c, *attr;
    const char *abs_key;
    int rc, col, abs_service_id, abs_oneway_flag, str_num, int_num;
    int req_serial = 0, rsp_serial, errcode;
    int n, m;
    abs_io_t *io;
    abs_v3_tbl_t *tbl;
    abs_v3_ent_type_t type;
    void *value;
    size_t lvalue;
    u_char *pdu, *data, ibuf[65536];
    size_t lpdu, ldata, iused = 0;

    /*
    ** Parse options 
    */

    /* Handle options */
    while((c = getopt(argc, argv, "dv")) != -1) {
        switch(c) {
          case 'd': debug++; break;
          case 'v':
            fprintf(stderr, "\nabsgw module " RADABS_VERSION ". "
               "Copyright (C) 2004 Golden Telecom (R).\n\n"
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
        fprintf(stderr, "absgw: unexpected command-line params\n\n"); usage(); 
    }

    /*
    ** Setup ABS connection
    */

    /* Create io session to local middleware server */
    if (abs_io_meta_create(&io, ABS_IO_TRANSPORT_STREAM, 1, -1, -1,\
                           NULL, NULL) < 0) {
      const char *ptr;
      while ((ptr = abs_io_log_pop(io, NULL)) != NULL)
        fprintf(stderr, "absgw: %s\n", ptr);
      exit(-1);
    }

    /* Request loop */
    fprintf(stderr, "absgw: ready for requests.\n");
    for(;;) {
        if ((rc = abs_v3_tbl_create(&tbl)) < 0) {
          fprintf(stderr, "absgw: -> abs_v3_tbl_create() failed: %s\n",
                  abs_v3_error_str(rc));
          break;
        }

        /*
         * Get the request from OpenRADIUS
         */

        /* Read header */
        if (read(0, msgbuf, 8) != 8) { perror("radabs: read"); break; }
        if (ntohl(msgbuf[0]) != 0xdeadbeef) {
            fprintf(stderr, "absgw: -> invalid magic 0x%08x!\n", 
                ntohl(msgbuf[0])); 
            break;
        }
        len = ntohl(msgbuf[1]);
        if (len < 8 || len > sizeof(msgbuf) - 4) {
            fprintf(stderr, "absgw: -> invalid length %d!\n", len); 
            break;
        }

        /* Read rest of message */
        if (read(0, msgbuf + 2, len - 8) != len - 8) {
            perror("absgw: read"); 
            break;
        }

        if (debug)
          fprintf(stderr, "absgw: -> %d-octet msg recved from radius core\n",
                  len-6);

        /*
         * Supported input attribute-value set:
         * 
         * int   -- ABS service ID
         * str   -- username w/o realm (uname)
         * str   -- realm w/o username (realm)
         * str   -- ABS resource name (rname)
         * ... a subset of standard RADIUS attrs
         *
         * output attr-value set:
         * int   -- ABS function return code
         * str   -- ABS function return message
         * ... a subset of standard RADIUS attrs
         */
        e = msgbuf + (len >> 2);
        col = 0; str_num = int_num = 0; abs_service_id = 0; abs_oneway_flag = 0;
        for(i = msgbuf + 2; i < e; i += ((len + 3) >> 2) + 4) {
            /* Get space/vendor/attribute/length */
            spc = ntohl(i[0]); vnd = ntohl(i[1]);
            atr = ntohl(i[2]); len = ntohl(i[3]);
            if (debug) {
                fprintf(stderr, "absgw: -> got space %d, vendor "
                        "%d, attribute %d, len %d\n",
                    spc, vnd, atr, len);
            }
            if (spc == C_DS_INTERNAL && vnd == 0 &&
                atr == C_DI_INT) {
                /* First INT goes for ABS service ID */
                if (!abs_service_id) {
                    if (len > sizeof(abs_service_id)) {
                        fprintf(stderr, "absgw: -> too large INT value\n");
                        exit(-1);
                    }
                    memcpy(&abs_service_id, (char *)(i + 4), len);
                    abs_service_id = ntohl(abs_service_id);
                    if (!abs_service_id) {
                        fprintf(stderr, "absgw: -> bad ABS service ID\n");
                        exit(-1);
                    }
                    if (debug) 
                      fprintf(stderr, "absgw: -> got ABS service ID %d\n",
                              abs_service_id);
                    continue;
                }
                abs_key = int_to_abs[int_num++];
                if (!abs_key) {
                    fprintf(stderr, "absgw: -> extra INT value\n");
                    exit(-1);
                }
                memcpy(&m, (char *)(i+4), len);
                m = htonl(m);
                if ((rc = abs_v3_tbl_set(tbl, 0, col, ABS_V3_ENT_TYPE_STRING,
                                         abs_key, -1)) == -1 ||
                    (rc = abs_v3_tbl_set(tbl, 1, col, ABS_V3_ENT_TYPE_INTEGER,
                                         &m, sizeof(m))) == -1) {
                    fprintf(stderr, "absgw: -> abs_v3_tbl_set() failed: %s",
                            abs_v3_error_str(rc));
                    exit(-1);
                }
                if (debug)
                  fprintf(stderr, "absgw: -> %s=%d\n", abs_key, m);
            }
            else if (spc == C_DS_INTERNAL && vnd == 0 &&
                     atr == C_DI_STR) {
                abs_key = str_to_abs[str_num++];
                if (!abs_key) {
                    fprintf(stderr, "absgw: -> extra STR value\n");
                    exit(-1);
                }
                if ((rc = abs_v3_tbl_set(tbl, 0, col, ABS_V3_ENT_TYPE_STRING,
                                         abs_key, -1)) == -1 ||
                    (rc = abs_v3_tbl_set(tbl, 1, col, ABS_V3_ENT_TYPE_STRING,
                                         (char *)(i+4), len)) == -1) {
                    fprintf(stderr, "absgw: -> abs_v3_tbl_set() failed: %s",
                            abs_v3_error_str(rc));
                    exit(-1);
                }
                if (debug) {
                  char t[4096];
                  strncpy(t, (char *)(i+4), len < sizeof(t) ? len : sizeof(t));
                  fprintf(stderr, "absgw: -> %s=%s\n", abs_key, t);
                }
            }
            else if (spc == C_DS_RAD_ATR && vnd == 0) {
                for(n = 0; attrmap[n].name; n++)
                    if (atr == attrmap[n].atrs)
                        break;
                if (!attrmap[n].name) {
                    fprintf(stderr, "absgw: -> unmapped attr in request %d\n", atr);
                    continue;
                }
                if ((rc = abs_v3_tbl_set(tbl, 0, col, ABS_V3_ENT_TYPE_STRING,
                                         attrmap[n].name, -1)) == -1) {
                    fprintf(stderr, "absgw: -> abs_v3_tbl_set() failed: %s",
                            abs_v3_error_str(rc));
                    exit(-1);
                }
                switch(attrmap[n].type) {
                case ABS_V3_ENT_TYPE_INTEGER:
                    memcpy(&m, (char *)(i+4), len);
                    m = htonl(m);
                    if ((rc = abs_v3_tbl_set(tbl, 1, col, attrmap[n].type,
                                             &m, len))) {
                        fprintf(stderr, "absgw: -> abs_v3_tbl_set() failed: %s",
                                abs_v3_error_str(rc));
                        exit(-1);
                    }
                    if (debug) {
                      fprintf(stderr, "absgw: -> %s=%d\n", attrmap[n].name, m);
                    }
                    break;
                case ABS_V3_ENT_TYPE_STRING:
                    if ((rc = abs_v3_tbl_set(tbl, 1, col, attrmap[n].type,
                                             (char *)(i+4), len))) {
                        fprintf(stderr, "absgw: -> abs_v3_tbl_set() failed: %s",
                                abs_v3_error_str(rc));
                        exit(-1);
                    }
                    if (debug) {
                      char t[4096];
                      strncpy(t, (char *)(i+4), len < sizeof(t) ? len : sizeof(t));
                      fprintf(stderr, "absgw: -> %s=%s\n", attrmap[n].name, t);
                    }
                    break;
                default:
                    fprintf(stderr, "absgw: -> unknown attribute type %d", 
                            attrmap[n].type);
                    exit(-1);
                }
            }
            col++;
        }

        if (!abs_service_id) {
            fprintf(stderr, "absgw: -> missing ABS service ID\n");
            exit(-1);
        }

        if ((rc = abs_v3_tbl_marshal(&pdu, &lpdu, tbl)) < 0) {
            fprintf(stderr, "absgw: -> abs_v3_tbl_marshal() failed: %s",
                    abs_v3_error_str(rc));
            exit(-1);
        }
    
        abs_v3_tbl_destroy(tbl);

        req_serial++;

        if ((rc = abs_msg_meta_marshal(&data, &ldata, &req_serial, 
                                       NULL, NULL, abs_service_id,
                                       abs_oneway_flag, 0, pdu, lpdu)) < 0) {
            fprintf(stderr, "absgw: -> abs_msg_meta_marshal() failed: %s",
                    abs_msg_error_str(rc));
            exit(-1);
        }

        free(pdu);

        /* Send message to server */
        rc = abs_io_tr_send(io, data, ldata);
        if (rc < 0 || rc != ldata) {
            const char *ptr;
            while ((ptr = abs_io_log_pop(io, NULL)) != NULL) {
                fprintf(stderr, "absgw: -> %s", ptr);
            }
            free(data);
            exit(-1);
        }

        free(data);

        if (debug)
          fprintf(stderr, "absgw: -> %d-octet msg sent to ABS\n", rc);

        /* Wait for new messages to come from server (blocking) */
        while(1) {
            rc = abs_io_tr_recv(io, ibuf+iused, sizeof(ibuf)-iused);
            if (rc <= 0) {
                const char *ptr;
                while ((ptr = abs_io_log_pop(io, NULL)) != NULL) {
                    fprintf(stderr, "absgw: <- %s", ptr);
                }
                free(data);
                exit(-1);
            }
            iused += rc;

            if (debug)
              fprintf(stderr, "absgw: <- %d-octet msg recved from ABS\n", rc);

            /* Watch for overflow XXX */
            if (iused == sizeof ibuf) {
                fprintf(stderr, "absgw: <- input overflown");
                exit(-1);
            }

            if ((rc = abs_msg_meta_demarshal(&rsp_serial, NULL, NULL, NULL,
                                             NULL, &errcode, &pdu, &lpdu,
                                             (u_char *)ibuf, iused)) < 0) {
                if (rc == ABS_MSG_ERR_INCOMPLETE) {
                  /* Keep waiting for new messages (or parts) to come */
                  continue;
                }

                fprintf(stderr, "absgw: <- abs_msg_meta_demarshal() failure: %s",
                        abs_msg_error_str(rc));
                exit(-1);
            }

            /* Strip message off the input */
            iused -= rc;
            memmove(ibuf, ibuf+rc, iused);

            if (rsp_serial != req_serial) {
                /* Might be late response */
              continue;
            }
          
            break;
        }

        len = 0;

        if (errcode != ABS_MSG_NETERR_NOERROR) {
            fprintf(stderr, "absgw: <- %s", abs_msg_neterr_str(errcode));
            if (pdu) 
                free(pdu);
            len = set_nak(msgbuf);
            if (write(1, msgbuf, len) != len) {
                perror("radldap: write");
                exit(-1);
            }
            continue;
        }

        if ((rc = abs_v3_tbl_demarshal(&tbl, pdu, lpdu)) < 0) {
            fprintf(stderr, "absgw: <- abs_v3_tbl_demarshal() failure: %s",
                    abs_v3_error_str(rc));
            exit(-1);
        }

        free(pdu);

        o = msgbuf + 2;

        for(col=0; ; col++) {
            /* Get attribute name from table header */
            if ((rc = abs_v3_tbl_get(tbl, 0, col, NULL, (void **)&attr, NULL)) < 0) {
                if (rc == ABS_V3_ERR_EMPTYCELL)
                  continue;
                if (rc == ABS_V3_ERR_OUTOFCOL || rc == ABS_V3_ERR_OUTOFROW)
                  break;

                fprintf(stderr, "absgw: <- abs_v3_tbl_get() failure: %s",
                        abs_v3_error_str(rc));
                abs_v3_tbl_destroy(tbl);
                exit(-1);
            }

            /* Get attribute value from first row's column */
            if ((rc = abs_v3_tbl_get(tbl, 1, col, &type, (void *)&value, \
                                     &lvalue)) < 0) {
                if (rc == ABS_V3_ERR_EMPTYCELL)
                    continue;
                if (rc == ABS_V3_ERR_OUTOFCOL || rc == ABS_V3_ERR_OUTOFROW)
                    break;

                fprintf(stderr, "absgw: <- abs_v3_tbl_get() failure: %s",
                        abs_v3_error_str(rc));
                abs_v3_tbl_destroy(tbl);
                exit(-1);
            }

			/* Check if there's room, considering current and new size */
            m = strlen(attr) + lvalue + 1;
			if (((o - msgbuf + 
			      4 + ((m + 3) >> 2)) << 2) >= C_MAX_MSGSIZE) {
                fprintf(stderr, "absgw: <- no more room for attribute %s\n",
                        attr);
				exit(-1);
			}

            switch(type) {
            case ABS_V3_ENT_TYPE_STRING:
              memcpy(&(o[4]), value, lvalue);
              if (debug) fprintf(stderr, "absgw: <- %s=%s\n", attr, (char *)value);
              break;
            case ABS_V3_ENT_TYPE_INTEGER:
              memcpy(&n, value, sizeof(n));
              o[4] = htonl(n);
              if (debug) fprintf(stderr, "absgw: <- %s=%d\n", attr, htonl(n));
              break;
            default:
              fprintf(stderr, "absgw: <- unknown value type %d in response attr %s\n",
                      type, attr);
              continue;
            }

            for(n = 0; attrmap[n].name; n++)
                if (!strcmp(attr, attrmap[n].name))
                    break;
            if (!attrmap[n].name) {
                fprintf(stderr, "absgw: <- unmapped attr in response %s\n", attr);
                continue;
            }

            /* Write attribute to message buffer and advance ptr */
            o[0] = htonl(attrmap[n].spcs);
            o[1] = htonl(attrmap[n].vnds);
            o[2] = htonl(attrmap[n].atrs);
            o[3] = htonl(m);
            o += 4 + ((m + 3) >> 2);
        }

        abs_v3_tbl_destroy(tbl);

        m = (o - msgbuf) << 2;
        msgbuf[1] = htonl(m);

        if (write(1, msgbuf, m) != m) { 
          perror("absgw: write");
          exit(-1);
        }

        if (debug)
          fprintf(stderr, "absgw: <- %d-octet msg sent to radius core\n", m);
    }

    signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, SIG_DFL);

    return 1;
}
