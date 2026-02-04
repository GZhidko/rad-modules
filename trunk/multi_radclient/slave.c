#include "slave.h"

int slave(slave_process_t * sp_info,
          char * path_to_executive,
	  char * argv[])
{
    pid_t pid;
    int ch_in[2];
    int ch_out[2];
    int ch_err[2];
    int *chanel[] = {ch_in, ch_out, ch_err};
    int i;
    int r;
    for (i=0; i<3; i++) {
        r = pipe(chanel[i]);
        if (r < 0) {
  	    write_error("Can not create pipe!");
  	    return -1;
        }
        write_debug("Create pipe #%d", i);
    }
    pid = fork();
    if (pid < 0) {
        write_error("Can not fork!");
        return -1;
    }
    if (pid == 0) { /* I'm child */
        close(ch_in[1]);
        close(ch_out[0]);
        close(ch_err[0]);
        dup2(ch_in[0], 0);
        dup2(ch_out[1], 1);
        dup2(ch_err[1], 2);
        execv(path_to_executive, argv);
        write_error("ERROR: exec() failed [Errno %d] %s: \"%s\"", errno, strerror(errno), argv[0]);
        close(ch_in[0]);
        close(ch_out[1]);
        close(ch_err[1]);
	exit(1);
    } else {
	write_debug("Create process PID=%d", pid);
	close(ch_in[0]);
	close(ch_out[1]);
	close(ch_err[1]);
	sp_info->pid = pid;
	sp_info->in = ch_in[1];
	sp_info->out = ch_out[0];
	sp_info->err = ch_err[0];
        return 0;
    }
    return -1; /* error, how can we get here?? */
}
