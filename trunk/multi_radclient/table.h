#ifndef __TABLE_H
#define __TABLE_H
#include "common.h"

#ifdef STD_OPENSSL
#include <openssl/lhash.h>
#else
#include "lhash.h"
#endif


typedef struct __config_record_t {
    char *key;
    second_key *first;
    char *second;
} config_record_t;


void init_table_first(void);
void init_table_second(void);
second_key *get_record_first(char *);
char *get_record_second(char *);
int reg_record_first(char *, second_key *);
int reg_record_second(char *, char *);
config_record_t* drop_record_first(char *);
config_record_t* drop_record_second(char *);
void clean_table(void);

void init_stat(void);
void print_stat(void);

#endif /* __TABLE_H */
