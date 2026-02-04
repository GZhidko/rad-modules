#include "proc.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processor_init(input_params_ctx *ctx) {
    int i;
    memset(ctx, 0, sizeof(input_params_ctx));
    for (i=0;i<PARAMS_NUMBER;i++) {
        ctx->argv[i] = malloc(ITEM_SIZE);
        if (!ctx->argv[i]) {
            write_error("malloc() failure");
            exit(1);
        }
    }
}

void processor_append(input_params_ctx *ctx, eater_ctx *c) {
    char error_buff[ITEM_SIZE];
    char dbuffk[ITEM_SIZE];
    char dbuffv[ITEM_SIZE];
    log_escape_string(c->key, c->key_len, dbuffk, ITEM_SIZE);
    log_escape_string(c->val, c->val_len, dbuffv, ITEM_SIZE);
    write_debug("append #%d key=[%s] val=[%s]", ctx->argc, dbuffk, dbuffv);

    if (ctx->argc == PARAMS_NUMBER) {
        write_error("CONTEXT OVERFLOW");
        exit(1);
    }
    if (c->key_len != 3) {
        log_escape_string(c->key, c->key_len, error_buff, ITEM_SIZE);
        write_warning("IGNORE ATTRIBUTE %s", error_buff);
        return;
    }
    if (memcmp("str", c->key, 3) != 0) {
        log_escape_string(c->key, c->key_len, error_buff, ITEM_SIZE);
        write_warning("IGNORE ATTRIBUTE %s (2)", error_buff);
        return;
    }
    if (c->val_len > ITEM_SIZE) {
        write_error("BUFFER OVERFLOW");
        exit(1);
    }

    memcpy(ctx->argv[ctx->argc], c->val, c->val_len);
    ctx->argvlen[ctx->argc] = c->val_len;
    ctx->argc++;
}

void processor_pop(input_params_ctx *ctx, 
                   int *argc, char ***argv, int **argvlen) {
  *argv = ctx->argv;
  *argc = ctx->argc;
  *argvlen = ctx->argvlen;

  ctx->argc = 0;
  return;
}
