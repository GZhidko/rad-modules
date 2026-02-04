#include "redis.h"
#include "proc.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

void redis_init(redis_ctx *rctx, 
                char *hostname, int port, int timeout) {
  struct timeval wait = { timeout/1000000, timeout%1000000 };

  rctx->native = redisConnectWithTimeout(hostname, port, wait);
  if (rctx->native == NULL || rctx->native->err) {
    if (rctx->native) {
      printf("Connection error: %s\n", rctx->native->errstr);
      redisFree(rctx->native);
    } else {
      printf("Connection error: can't allocate redis context\n");
    }
    exit(1);
  }
  write_debug("connected to Reddis server at %s:%i\n", hostname, port);
}

void redis_process(redis_ctx *rctx, int argc, char **argv, size_t argvlen[], 
                  char *obuff, size_t obufflen) {
    redisReply *reply;

    reply = redisCommandArgv(rctx->native, argc, (const char **)argv, (const size_t *)argvlen);
    if (!reply) {
        write_error("redis error: %s", reply->str);
        exit(1);
    }

    switch(reply->type) {
    case REDIS_REPLY_ERROR:
      write_error("redis error: %s (%lld)", reply->str, reply->integer);
      exit(-1);
    case REDIS_REPLY_STATUS:
      snprintf(obuff, obufflen, "str=\"%s\"\nint=0\n\n", reply->str);
      break;
    case REDIS_REPLY_STRING:
      snprintf(obuff, obufflen, "str=\"%s\"\nint=0\n\n", reply->str);
      break;
    case REDIS_REPLY_INTEGER:
      snprintf(obuff, obufflen, "str=\"%li\"\nint=0\n\n", reply->integer);
      break;
    default:
      write_warning("unknown redis type in response %i", reply->type);
      snprintf(obuff, obufflen, "int=1\n\n");
      break;
    }

    freeReplyObject(reply);
}
