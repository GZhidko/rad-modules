#ifndef __DISPATCH_H
#define __DISPATCH_H

#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include "log.h"
#include "slave.h"
#include "common.h"

#define IO_BUFFER_SIZE 16384
#define IO_SLOT_NUMBER 33
#define IO_WRITE_TIMEOUT 5

#define IO_READ_STDOUT      0x01
#define IO_READ_ZERO_STDOUT 0x02
#define IO_READ_STDERR      0x04
#define IO_READ_ZERO_STDERR 0x08
#define IO_EXC_STDOUT       0x10
#define IO_EXC_STDERR       0x20
#define IO_EXC_STDIN        0x40
#define IO_EXC_WRITE_TO     0x80

#define IO_ERROR            0xf6

extern time_t global_timestamp;

typedef struct io_stream {
    int descriptor;
    char buffer[IO_BUFFER_SIZE];
    char accum[IO_BUFFER_SIZE];
    char *last;
} io_stream_t;

typedef struct io_slot {
    pid_t pid;
    int active;
    union {
        struct {
	    io_stream_t in;
	    io_stream_t out;
	    io_stream_t err;
        } byname;
        io_stream_t byid[3];
    } stream; /* in, out, err */
    int (*processor)(struct io_slot *, int, int);
    time_t last_write_time;
} io_slot_t;

void io_init(void (*timeproc)(void));

void io_unset_slot(int id);

void io_set_slot(int id,
                 pid_t pid,
		 int in,
	  	 int out,
  		 int err,
		 int (*processor)(struct io_slot *, int, int));

int io_write(int id,
             void *data,
             ssize_t len);

void io_flags_to_streing(char *, int);

void dispatch_forever(void);

#endif /* __DISPATCH_H */
