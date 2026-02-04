#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stor.h"
#include "log.h"

#define CFG_BUFFSIZE 1024

struct re_storage_struct re_storage[SLOTS_IN_STORAGE];

void find_end_and_start_of_line (char *s, char **ss, char **sf) {
  *ss = NULL;
  *sf = NULL;
  while (*s) {
    if (*s != '\040' && *s != '\t' && *s != '\n') {
      *sf = s;
      if (*ss == NULL) *ss = s;
    }
    s++;
  }
}

void append_string(char ** s, char * a) {
  int l1, l2;
/*
  if (debug) write_error("Append string \"%s\"\n", a);
*/
  l1 = strlen(*s);
  l2 = strlen(a);
  *s = realloc(*s, l1 + l2 + 1);
  if (*s == NULL) {
    write_error("Can not reallocate memory\n");
    exit(1);
  }
  strcat(*s, a);
}

void radre_stor_load (char ** files) {
  FILE *fh;
  char buff[CFG_BUFFSIZE];
  char *ss, *sf, *s, *p;
  int i, n, c;
  int curr_slot = -1;
  size_t len, reply_len;

  if (debug) write_error("Reading configuration...\n");
  while (*files) {
    if (debug) write_error("Load file: %s\n", *files);
    fh = fopen(*files, "r");
    if (fh == NULL) {
      write_error("Can not open file %s for reading.\n", *files);
      exit(1);
    }
    while(fgets(buff, CFG_BUFFSIZE, fh)) {
      switch (*buff) {
        case '\n':
        case '#':
          break;
        case '\040':
        case '\t':
          if (curr_slot < 0) {
            write_error("ERROR: Error in configuration file: section without head.\n");
            exit(1);
          }
          find_end_and_start_of_line(buff, &ss, &sf);
          if (ss == NULL || sf == NULL) {
            write_error("WARNING: Left line: contain spaces only.\n");
            break;
          }
          *(sf+1) = '\n';
          *(sf+2) = '\0';
          append_string(&(re_storage[curr_slot].reply), ss);
          break;
        default:
          curr_slot ++;
          if (curr_slot >= SLOTS_IN_STORAGE) {
            write_error("ERROR: Slots run out (limit = %d).\n",
                        (int)(SLOTS_IN_STORAGE));
            exit(1);
          }
          find_end_and_start_of_line(buff, &ss, &sf);
          len = sf - buff + 2; /* + '\0' */
          s = malloc(len);
          if (s == NULL) {
            write_error("Can not allocate memory\n");
            exit(1);
          }
          *(sf+1) = '\0';
          memcpy(s, buff, len);
          re_storage[curr_slot].regexpr = s;
          if (debug) write_error("Open slot #%d for RE=\"%s\"\n", curr_slot, s);
          re_storage[curr_slot].reply = malloc(1);
          *re_storage[curr_slot].reply = '\0';
          reply_len = 0;
          break;
      }
    }
    fclose(fh);
    files ++;
  }

  if (debug) write_error("Compilation...\n");
  for (i=0; i<=curr_slot; i++) {
    if (debug) write_error("Slot %d\n", i);
    append_string(&(re_storage[i].reply), "int=1\n\n");
    re_storage[i].regex = malloc(sizeof(regex_t));
    if (regcomp(re_storage[i].regex, re_storage[i].regexpr, REG_EXTENDED)) {
      write_error("REGEX: Compilation error. ER=\"%s\"\n",
                 re_storage[i].regexpr);
    }
    /* preprocess replacements */
    n = 0;
    for(p = re_storage[i].reply, c = 0; *p; p++) {
      if (*p == '\\') {
        if (c == 1) {
          for(s = p+1; *s; s++) *(s-1)=*s; /* drop one slash */
          *(s-1) = '\0';
          c = 0;
        } else {
          c++;
        }
      } else {
        if (c == 1) { /* XXX process only one digit! */
          if ('0' <= *p && *p <= '9') {
            re_storage[i].replacements[n].so = p - 1;
            re_storage[i].replacements[n].eo = p + 1;
            re_storage[i].replacements[n].n = *p - '0';
            n++;
            if (n >= MAX_REPLACEMENTS) {
              write_error("REPLACEMENT: Too much (MAX_REPLACEMENTS=%d)\n",
                MAX_REPLACEMENTS);
              exit(1);
            }
          }
          c = 0;
        }
      }
    }
    re_storage[i].replacements[n].so = NULL;
    re_storage[i].replacements[n].eo = NULL;
    re_storage[i].replacements[n].n = -1;
    /* /replacements */
    if (debug) write_error("Pair %d created\n"
                           "RE: \"%s\":\n%s",
                           i,
                           re_storage[i].regexpr,
                           re_storage[i].reply);
  }
  re_storage[i].regex = NULL;
  re_storage[i].reply = NULL;
  re_storage[i].regexpr = NULL;
}

void radre_search(char * s, char *r) {
  struct re_storage_struct * rp;
  struct re_palces *pp;
  regmatch_t re_groups[MAX_GROUPS];
  char *p, *so, *eo;
  int start;
  rp = re_storage;
  while (rp->regex) {
    if (debug) write_error("Try to match \"%s\" to \"%s\"\n", s, rp->regexpr);
    /* it is good idea to use re_groups=NULL and flag REG_NOSUB,
       if re_groups empty? or not? */
    if (regexec(rp->regex, s, MAX_GROUPS, re_groups, 0) == 0) {
      if (debug) write_error("Match \"%s\" to \"%s\"\n", s, rp->regexpr);
      /* XXX test overbuffer *r!!! */
      if (rp->replacements->n == -1) {
        strcpy(r, rp->reply);
        return;
      }
      p = rp->reply;
      for (pp = rp->replacements; pp->n != -1; pp++) {
        while (p < pp->so) *(r++) = *(p++); /* copy from .reply */
        p = pp->eo; /* left '\N' */
        start = re_groups[pp->n].rm_so; /* copy match */
        if (start < 0) {
          write_error("HAS NOT GROUP FOR MATCH\n");
        } else {
          so = s + re_groups[pp->n].rm_so;
          eo = s + re_groups[pp->n].rm_eo;
          while (so < eo) {
            *(r++) = *(so++);
          }
        }
      }
      while (*p) {
        *(r++) = *(p++);
      }
      *r = '\0';
      return;
    }
    rp++;
  }
  strcpy(r, "int=0\n\n");
}
