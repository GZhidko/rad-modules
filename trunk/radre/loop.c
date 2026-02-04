#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "log.h"
#include "stor.h"
#include "../stats_c.h"

extern const char *stats_file;
extern int stats_interval_min;
#define RQBUFFSIZE 4096
#define RPBUFFSIZE 4096
#define BUFFSIZE 1024

/* states of deterministic finite-state machine (DFA):
   F -- passive (search for '\n')
   S -- we are in line (search for 's', left '\t' and '\n')
   s,
   t,
   r -- letters
   A -- optional spaces before '='
   = -- is it
   B -- optional spaces after '='
   K -- reading string
   C -- check '\n' after last '"'
 */

char * get_str(char *ibuf, char *ilim, char *obuf) {
  char *i, *o;
  int ok = 0;
  char st = 'S';
  o = obuf;
  for (i = ibuf; i < ilim && ok == 0; i++) {
    switch (st) {
      case 'F': if (*i == '\n') st = 'S';
                break;
      case 'S': switch (*i) {
                  case '\n':
                  case '\t': break;
                  case 's': st = 's'; break;
                  default:  st = 'F'; break;
                } break;
      case 's': if (*i == 't') st = 't';
                else           st = 'F';
                break;
      case 't': if (*i == 'r') st = 'r';
                else           st = 'F';
                break;
      case 'r':
      case 'A': switch (*i) {
                  case ' ': st = 'A'; break;
                  case '=': st = '='; break;
                  default:  st = 'F'; break;
                } break;
      case '=':
      case 'B': switch (*i) {
                  case ' ': st = 'B'; break;
                  case '"': st = 'K'; break;
                  default:  st = 'F'; break;
                } break;
      case 'K': if (*i == '"') st = 'C';
                else           *o++ = *i;
                break;
      case 'C': if (*i == '\n') *o = '\0', ok = 1;
                else            o = obuf, st = 'F';
                break;
    }
  }
  return i;
}

void reply(char *s, char *slim) {
  char repbuff[RPBUFFSIZE];
  char *r;
  char str[BUFFSIZE];
  //char rep[BUFFSIZE];
  size_t l, len, wlen;

  if ((s = get_str(s, slim, str)) < slim) { /* get only first element */
    if (debug) write_error("match=\"%s\"\n", str);
    radre_search(str, repbuff);
    write(1, repbuff, strlen(repbuff));
    if (debug) write_error("reply=\"%s\"\n", repbuff);
  } else {
    write(1, "int=1\n\n", 7);
  }
}

void main_loop() {
  char rq[RQBUFFSIZE], *buff, *nbuff, *rq_start, *rq_end, *a;
  int nl_cnt, taillen;
  size_t n, l;
  stats_c_t stats;
  stats_c_init(&stats, "radre", stats_file, stats_interval_min);
  buff = rq;
  nl_cnt = 0;
  rq_start = rq;
  for (;;) {
    l = buff - rq;
    if (l >= RQBUFFSIZE) {
      write_error("INLET OVERFILL (l=%d)\n", l);
      exit(1);  /* XXX SERVER DIE */
    }
    n = read(0, buff, RQBUFFSIZE - l);
    if (n == 0) {
      write_error("Server close STDIN (server die?)\n");
      exit(1);  /* XXX SERVER DIE */
    }
    if (n < 0) {
      write_error("STDIN error: %s\n", strerror( errno ));
      exit(1);  /* XXX SERVER DIE */
    }
    nbuff = buff + n;
    for (a=buff; a<nbuff; a++) {
      if (*a == '\n') nl_cnt++; else nl_cnt=0;
      if (nl_cnt == 2) {
        rq_end = a + 1;
        uint64_t stats_start_ms = stats_c_now_ms();
        reply(rq_start, rq_end);
        stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
        rq_start = rq_end;
        nl_cnt = 0;
      }
    }
    if (rq_start != rq) {
      memmove(rq, rq_end, nbuff - rq_end);
      rq_start = rq;
      buff = rq + (nbuff - rq_end);
    } else {
      buff = nbuff;
    }
  }
}
