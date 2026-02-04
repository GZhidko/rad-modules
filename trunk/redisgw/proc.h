#ifndef __PROC_H
#define __PROC_H

#include "parse.h"
#include "proc.h"

#define INPUT_BUFFER_SIZE 8192
#define PARAMS_NUMBER 32
#define ITEM_SIZE 256

typedef struct input_params_ {
    char *argv[PARAMS_NUMBER];
    int argc;
    int argvlen[PARAMS_NUMBER];
} input_params_ctx;

void processor_init(input_params_ctx * ctx);
void processor_append(input_params_ctx * ctx, eater_ctx * c);
void processor_pop(input_params_ctx * ctx, int *argc, char ***argv, int **argvlen);

#endif
