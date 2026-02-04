#ifndef __REDIS_H
#define __REDIS_H

#include <hiredis/hiredis.h>

typedef struct redis_ctx_ {
  redisContext *native;
} redis_ctx;

void redis_init(redis_ctx *ctx, char *hostname, int port, int timeout);
void redis_process(redis_ctx *ctx, int argc, char **argv, size_t argvlen[], char *obuff, size_t obufflen);

// void redis_shutdown(redis_ctx *ctx);

#endif
