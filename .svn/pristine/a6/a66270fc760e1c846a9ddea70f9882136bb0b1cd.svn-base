#include <regex.h>
#define SLOTS_IN_STORAGE 512
#define MAX_REPLACEMENTS 20
#define MAX_GROUPS       10

struct re_palces {
  char * so;
  char * eo;
  int n;
};

struct re_storage_struct {
  regex_t * regex;
  char * reply;
  char * regexpr;
  struct re_palces replacements[MAX_REPLACEMENTS];
};

void radre_stor_load(char **);

void radre_search(char *, char *);
