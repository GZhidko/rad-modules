#include "dispatch.h"

io_slot_t slot[IO_SLOT_NUMBER];

void (*timeout_processor)(void) = NULL;

void io_flags_to_streing(char *b, int f) {
    char s[] = "\0\0";
    *b = '\0';
    if (f & IO_READ_STDOUT)  { strcat(b, s); *s=','; strcat(b, "READ_STDOUT"); }
    if (f & IO_READ_ZERO_STDOUT) { strcat(b, s); *s=','; strcat(b, "READ_ZERO_STDOUT"); }
    if (f & IO_READ_STDERR)  { strcat(b, s); *s=','; strcat(b, "READ_STDERR"); }
    if (f & IO_EXC_STDOUT)   { strcat(b, s); *s=','; strcat(b, "EXC_STDOUT"); }
    if (f & IO_EXC_STDERR)   { strcat(b, s); *s=','; strcat(b, "EXC_STDERR"); }
    if (f & IO_EXC_STDIN)    { strcat(b, s); *s=','; strcat(b, "IO_EXC_STDIN"); }
    if (f & IO_EXC_WRITE_TO) { strcat(b, s); *s=','; strcat(b, "IO_EXC_WRITE_TO"); }
}

void io_init(void (*timeproc)(void)) {
    int i;
    for(i = 0; i < IO_SLOT_NUMBER; i++) {
	io_unset_slot(i);
    }
    timeout_processor = timeproc;
}

void io_unset_slot(int id) {
    slot[id].active = 0;
}

int io_write(int id,
             void *data,
             ssize_t len) {
    io_slot_t *s;
    io_stream_t *w;
    s = slot + id;
    w = &s->stream.byname.in;
    if (w->last - w->buffer + len > IO_BUFFER_SIZE) {
        write_error("Slot %d: Buffer overflow.", id);
        return -2;
    }
    memcpy(w->last, data, len);
    w->last += len;
    s->last_write_time = global_timestamp;
    return 0;
}

void io_set_slot(int id,
                 pid_t pid,
                 int in,
		 int out,
		 int err,
                 int (*processor)(struct io_slot *, int, int)) {
    int i;
    io_stream_t *stm;

    write_info("io: add slot %d (%d/%d/%d)", id, in, out, err);
    io_slot_t *s;
    s = slot + id;
    s->active = 1;
    s->pid = pid;
    for(i = 0; i < 3; i++) {
	stm = s->stream.byid + i;
	stm->last = stm->buffer;
	*stm->accum = '\0';
    }
    (s->stream.byid + 0)->descriptor = in;
    (s->stream.byid + 1)->descriptor = out;
    (s->stream.byid + 2)->descriptor = err;
    s->processor = processor;
}

void dispatch_forever() {
    char XXX[4000];
    int i;
    io_slot_t *ios;
    io_stream_t *strm;
    fd_set readfds;
    fd_set writefds;
    fd_set errorfds;
    struct timeval tv;
    int fd_set_size;
    int d;
    ssize_t len, alen;
    int raise_processor_flag;
    time_t last_timeout = 0;

    for(;;) {
	/* collect descriptors */
	/* ------------------- */
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);
	fd_set_size = 0;
	write_debug("Enter to loop; prepare selection:");
        for(i = 0; i < IO_SLOT_NUMBER; i++) {
	    ios = slot + i;
	    if (ios->active) {
	        write_debug("Check slot %d (add stdout and stderr)", i);
		d = ios->stream.byname.out.descriptor;
		FD_SET(d, &readfds);
		FD_SET(d, &errorfds);
                fd_set_size = d > fd_set_size ? d : fd_set_size;
		d = ios->stream.byname.err.descriptor;
		if (d >= 0) { /* do not disp. stderr */
		FD_SET(d, &readfds);
		FD_SET(d, &errorfds);
		fd_set_size = d > fd_set_size ? d : fd_set_size;
		}
		if (ios->stream.byname.in.last !=
                    ios->stream.byname.in.buffer) {
  	            write_debug("Add stdin slot=%d", i);
                    if (global_timestamp - ios->last_write_time > IO_WRITE_TIMEOUT) {
			write_error("%d %d %d", global_timestamp, 
				    ios->last_write_time, i);
		        ios->processor(slot, i, IO_EXC_WRITE_TO);
                    } else {
		        d = ios->stream.byname.in.descriptor;
		        FD_SET(ios->stream.byname.in.descriptor, &writefds);
		        fd_set_size = d > fd_set_size ? d : fd_set_size;
                    }
		}
	    }
	}
	/* select */
	/* ------ */
	write_debug("select...");
        tv.tv_sec = 12;
        tv.tv_usec = 500000;
        select(fd_set_size+1, &readfds, &writefds, &errorfds, &tv);
	write_debug("selected");
        global_timestamp = time(&global_timestamp);
	/* timing */
	/* ------ */
        if (timeout_processor != NULL) {
            if (global_timestamp - last_timeout > exp_time) timeout_processor();
        } else {
            write_error("Set up timeout_processor!");
        }
	/* reading and processing */
	/* ---------------------- */
	for(i = 0; i < IO_SLOT_NUMBER; i++) {
	    ios = slot + i;
	    raise_processor_flag = 0;
	    if (ios->active) {
		d = ios->stream.byname.out.descriptor;
		if (FD_ISSET(d, &readfds)) {
		    strm = &ios->stream.byname.out;
		    len = read(d, strm->buffer, IO_BUFFER_SIZE);
		    *(strm->buffer + len) = '\0';
		    
		    write_debug("READ stdout: <%s>", strm->buffer);

		    write_debug("read %d bytes from slot %d", len, i);
		    strm->last = strm->buffer + len;
		    raise_processor_flag |= IO_READ_STDOUT;
		    if (len == 0) {
		        raise_processor_flag |= IO_READ_ZERO_STDOUT;
		    }
		}
		if (FD_ISSET(d, &errorfds)) {
		    raise_processor_flag |= IO_EXC_STDOUT;
		}
		d = ios->stream.byname.err.descriptor;
		if (d >= 0) { /* d<1 -- do not dispatch error strem */
 		if (FD_ISSET(d, &readfds)) {
		    strm = &ios->stream.byname.err;
		    len = read(d, strm->buffer, IO_BUFFER_SIZE);
		    *(strm->buffer + len) = '\0';
		    
		    write_debug("READ stderr: <%s>", strm->buffer);

		    write_debug("read ERR %d bytes from slot %d", len, i);
		    strm->last = strm->buffer + len;
		    if (len == 0) {
		     /* raise_processor_flag |= IO_READ_ZERO_STDERR; */
			ios->stream.byname.err.descriptor = -1;
		    } else {
  		        raise_processor_flag |= IO_READ_STDERR;
		    }
		}
		if (FD_ISSET(d, &errorfds)) {
		    raise_processor_flag |= IO_EXC_STDERR;
		}
		}
		d = ios->stream.byname.in.descriptor;
		if (FD_ISSET(d, &writefds)) {
		    strm = &ios->stream.byname.in;
		    alen = strm->last - strm->buffer;
		    log_escape_string(strm->buffer, alen, XXX, 4000);
		    len = write(d, strm->buffer, alen);
		    if (len != alen) {
		        memcpy(strm->buffer, strm->buffer+len, alen-len);
		    }
		    strm->last -= len;
		    write_debug("write %d/%d slot=%d \"%s\"", len, alen, i, XXX);
		}
		if (FD_ISSET(d, &errorfds)) {
		    raise_processor_flag |= IO_EXC_STDIN;
		}
		if (raise_processor_flag) {
		    write_debug("Call processor. slot=%d. reason=0x%04X.", i, raise_processor_flag);
		    ios->processor(slot, i, raise_processor_flag);
		}
	    }
	}
    }
}
