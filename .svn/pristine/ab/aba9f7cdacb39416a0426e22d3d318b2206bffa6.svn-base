#ifndef __MULTY_RADCLIENT_H
#define __MULTY_RADCLIENT_H

#include "common.h"
#include "dispatch.h"
#include "table.h"
#include "args.h"

void parse_prog(void);
void init_client(int);

int processor(io_slot_t slots[], int, int);
int process_main(char*);
int process_radclient(char*, int);

void work_client(attr*, char*, char*, char*, char*, char*);
void work_main(attr*, char*, char*);

void init_attr(attr*);
void destroy_attr(attr*);

void print_packet(attr*);

int create_out_key(char*, attr*, int);

int check_triplet(char*, char*, char*);

#endif /* __MULTY_RADCLIENT_H */
