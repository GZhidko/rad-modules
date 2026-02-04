#include "proc.h"
#include "ms_mppe.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _processor_prepare(input_params_ctx * ctx) {
    ctx->state = 0;
}

void processor_init(input_params_ctx * ctx, int mode) {
    ctx->processor_mode = mode;
    ctx->buffers[0].buff = ctx->secret;
    ctx->buffers[0].size = SECRET_SIZE;
    ctx->buffers[0].len  = &ctx->secret_len;
    ctx->buffers[1].buff = ctx->authen;
    ctx->buffers[1].size = 16;
    ctx->buffers[1].len  = &ctx->mppe_len; /* last item */
    ctx->buffers[2].buff = ctx->mppe;
    ctx->buffers[2].size = MPPE_SIZE;
    ctx->buffers[2].len  = &ctx->mppe_len;
    _processor_prepare(ctx);
}

void processor_append(input_params_ctx * ctx, eater_ctx * c) {
    char error_buff[200];

    char dbuffk[4000];
    char dbuffv[4000];
    log_escape_string(c->key, c->key_len, dbuffk, 4000);
    log_escape_string(c->val, c->val_len, dbuffv, 4000);
    write_debug("append #%d key=[%s] val=[%s]", ctx->state, dbuffk, dbuffv);

    if (ctx->state == PARAMS_NUMBER) {
        write_error("CONTEXT OVERFLOW");
        exit(1);
    }
    if (c->key_len != 3) {
        log_escape_string(c->key, c->key_len, error_buff, 200);
        write_warning("IGNORE ATTRIBUTE %s", error_buff);
        return;
    }
    if (memcmp("str", c->key, 3) != 0) {
        log_escape_string(c->key, c->key_len, error_buff, 200);
        write_warning("IGNORE ATTRIBUTE %s (2)", error_buff);
        return;
    }
    if (ctx->buffers[ctx->state].size < c->val_len) {
        write_error("BUFFER OVERFLOW");
        exit(1);
    }

    memcpy(ctx->buffers[ctx->state].buff, c->val, c->val_len);
    *ctx->buffers[ctx->state].len = c->val_len;

    ctx->state++;
}


void _processor_check_ready(input_params_ctx * ctx) {
    if (ctx->state != PARAMS_NUMBER) {
        write_error("INVALID CONTEXT (state=%d)", ctx->state);
        exit(1);
    }
}

void processor_process(input_params_ctx * ctx) {
    char buff[4000];
    char obuff[4000];
    _processor_check_ready(ctx);

    log_escape_string(ctx->mppe, ctx->mppe_len, buff, 4000);
    write_info("I: mppe_len=%d; mppe=\"%s\"", ctx->mppe_len, buff);

    if (ctx->processor_mode == PROCESSOR_MODE_ENCODE) {

        ms_mppe_encrypt_value(
            (unsigned char *)ctx->mppe, ctx->mppe_len,
            (unsigned char *)ctx->secret, ctx->secret_len,
            (unsigned char *)ctx->authen);

    } else {

        ms_mppe_decrypt_value(
            (unsigned char *)ctx->mppe, ctx->mppe_len,
            (unsigned char *)ctx->secret, ctx->secret_len,
            (unsigned char *)ctx->authen);

    }

    log_escape_string(ctx->mppe, ctx->mppe_len, buff, 4000);
    write_info("O: mppe_len=%d; mppe=\"%s\"", ctx->mppe_len, buff);

    snprintf(obuff, 4000, "str=\"%s\"\nint=0\n\n", buff);
    write(1, obuff, strlen(obuff));

    _processor_prepare(ctx);
}

