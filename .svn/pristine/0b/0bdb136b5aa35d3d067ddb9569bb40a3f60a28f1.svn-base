#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef STD_OPENSSL
#include <openssl/lhash.h>
#else
#include "lhash.h"
#endif
#include "log.h"

typedef struct __config_record_t {
  char *key;
  char *text;
} config_record_t;

LHASH *config_hash;

void append_text(config_record_t *a, char *text) {
  a->text = realloc(a->text, strlen(a->text) + strlen(text) + 1);
  if (a->text == NULL) {
    write_error("REALLOC ERROR!\n");
    exit(1);
  }
  strcat(a->text, text);
}

unsigned long lh_hash_cb(config_record_t *r) {
  char *a;
  unsigned long h = 0;
  int c = 0;
  for (a = r->key; *a && c < 32; a++, c++) {
    h ^= (unsigned long)(*a);
    h <<= 1;
  }
  return h;
}

int lh_cmp_cb(config_record_t *a, config_record_t *b) {
  return strcmp(a->key, b->key);
}

void lh_dump_all_cb(config_record_t *a) {
  write_error("KEY: %s\nVALUE:\n%s\n", a->key, a->text);
}

void lh_fix_all_cb(config_record_t *a) {
  append_text(a, "int=1\n\n");
}

#ifdef STD_OPENSSL
static IMPLEMENT_LHASH_HASH_FN(lh_hash_cb, config_record_t *);
static IMPLEMENT_LHASH_COMP_FN(lh_cmp_cb, config_record_t *);
static IMPLEMENT_LHASH_DOALL_FN(lh_dump_all_cb, config_record_t *);
static IMPLEMENT_LHASH_DOALL_FN(lh_fix_all_cb, config_record_t *);
#endif

int reg_record(char *key, char *text) {
  config_record_t a, *b;
  void *p;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(config_hash, &a);
#else
  b = (config_record_t *)lh_retrieve(config_hash, (char *)&a);
#endif
  if (b == NULL) {
    b = malloc(sizeof(config_record_t));
    b->key  = malloc(strlen(key)+1);
    b->text = malloc(strlen(text)+1);
    strcpy(b->key, key);
    strcpy(b->text, text);
#ifdef STD_OPENSSL
    lh_insert(config_hash, b);
#else
    lh_insert(config_hash, (char *)b);
#endif
  } else {
    append_text(b, text);
  }
}

char *get_record(char *key) {
  config_record_t a, *b;
  a.key = key;
#ifdef STD_OPENSSL
  b = lh_retrieve(config_hash, &a);
#else
  b = (config_record_t *)lh_retrieve(config_hash, (char *)&a);
#endif
  if (b == NULL) {
    return "int=0\n\n";
  } else {
    return b->text;
  }
}

void fix_table() {
#ifdef STD_OPENSSL
  lh_doall(config_hash, LHASH_DOALL_FN(lh_fix_all_cb));
#else
  lh_doall(config_hash, lh_fix_all_cb);
#endif
}

void dump_table() {
#ifdef STD_OPENSSL
  lh_doall(config_hash, LHASH_DOALL_FN(lh_dump_all_cb));
#else
  lh_doall(config_hash, lh_dump_all_cb);
#endif
}

void init_table() {
#ifdef STD_OPENSSL
  config_hash = lh_new(LHASH_HASH_FN(lh_hash_cb), LHASH_COMP_FN(lh_cmp_cb));
#else
  config_hash = lh_new(lh_hash_cb, lh_cmp_cb);
#endif
}
