#ifndef __PROC_H
#define __PROC_H

#include "parse.h"

#define INPUT_BUFFER_SIZE 1024
#define SECRET_SIZE 64
#define MPPE_SIZE 256
#define PARAMS_NUMBER 3

#define PROCESSOR_MODE_ENCODE 0
#define PROCESSOR_MODE_DECODE 1

typedef struct input_params_ {
    int state;
    int processor_mode;
    struct {
        char * buff;
        int size;
        int * len;
    } buffers[PARAMS_NUMBER];
    char secret[SECRET_SIZE];
    int  secret_len;
    char authen[16];
    char mppe[MPPE_SIZE];
    int  mppe_len;
} input_params_ctx;

void processor_init(input_params_ctx * ctx, int mode);

void processor_append(input_params_ctx * ctx, eater_ctx * c);

void processor_process(input_params_ctx * ctx);

#endif
