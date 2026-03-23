#include "multi_radclient.h"
#include "../stats_c.h"
#include <stdlib.h>

/*
Counter for id
*/
int cid;
static stats_c_t stats;

void print_packet(attr *out) {
    attr *p;
    static const char *func = "print_packet";
    
    write_debug("%s: packet from openradius", func);
    for(p = out; p != NULL; p = p->next) {
	write_debug("s:%s, v:%s, n:%s, t:%s, val:%s", 
	       p->space, p->vendor,
	       p->name, p->type, 
	       p->value);
    }
}

int create_out_key(char *key, attr *out, int radclnt) {
    attr *p;
    char id[4];
    int flag = 0;

    for(p = out; p != NULL; p = p->next) {
	if (strcmp(p->name, "RAD-Identifier") == 0) {
	    flag = 1;
	    strncpy(id, p->value, sizeof(id));
	}
    }
    
    if (flag == 0) {
	return 1;
    }    
    sprintf(key, "%d_%s", radclnt, id);

    return 0;
}

void work_client(attr *out, char* out_key, char* buf, 
		  char* ip, char* port, char* idd) {
    attr *s;
    int id;
    int radclnt;
    char *p, *c;
    char *packet;
    char str[1024];
    int i;
    static const char *func = "work_client";
    
    write_debug("%s: prepare packet for radclient", func);
    packet = (char*)malloc(PACKAGE_SIZE * sizeof(char));
    *packet = '\0';

    if (!packet) {
	write_error("%s: start radclient: malloc", func);
    }
    
    c = out_key;    
    while(*c != '_') {
	c++;
    }    
    *c = '\0';
    p = c + 1;    
    radclnt = atoi(out_key);
    id = atoi(p);
    
    /* Change RAD-Identifier */    

    write_debug("%s: client = %d slot = %d for [ip = %s port = %s id = %s]", 
	       func, radclnt, id, ip, port, idd);
	    
    for(s = out; s != NULL; s = s->next) {
	if (!strcmp(s->name, "RAD-Authenticator")) {
	    continue;
	}
	*str = '\0';
	if (strcmp(s->space, "")) {
	    strncat(str, s->space, strlen(s->space) + 1);
	    strcat(str, ":");	    
	}	
	if (strcmp(s->vendor, "")) {
	    strncat(str, s->vendor, strlen(s->vendor));
	    strcat(str, ":");
	}	
	strncat(str, s->name, strlen(s->name));
	strcat(str, "=");
	if (strcmp(s->type, "")) {
	    strncat(str, s->type, strlen(s->type));
	    strcat(str, ":");
	}	
	if (!strcmp(s->name, "RAD-Identifier")) {
	    strncat(str, p, strlen(p));
	} else {
	    strncat(str, s->value, strlen(s->value));
	}		
	strcat(str, "\n");
	strncat(packet, str, strlen(str));
    } 
    strcat(packet, "\n");
    /* Copy packet to buffer for radclient */    
    strcpy(buf, packet);
    
    /* Free packet */
    free(packet);
}

void work_main(attr *out, char* in_key, char *buf) {
    attr *p;
    char *packet;
    char str[1024];
    int i;
    char *p_ip, *p_port, *p_id;
    char ip[16];
    char port[6];
    char id[4];    
    int flag = 1;
    static const char *func = "work_main";

    *ip = *port = *id = '\0';
    
    p_ip = ip;
    p_port = port;
    p_id = id;    
    
    write_debug("%s: prepare packet for openradius", func);
    packet = (char*)malloc(PACKAGE_SIZE * sizeof(char));
    *packet = '\0';

    if (!packet) {
	write_error("%s: create_out_packet: malloc", func);
    }
    
    for(i = 0; i < strlen(in_key); i++) {
	if (in_key[i] == '_') {
	    flag++;
	} else if (flag == 1) {
	    *p_ip = in_key[i];
	    p_ip++;
	    *p_ip = '\0';
	} else if (flag == 2) {
	    *p_port = in_key[i];
	    p_port++;
	    *p_port = '\0';
	} else if (flag == 3) {
	    *p_id = in_key[i];
	    p_id++;
	    *p_id = '\0';
	}	
    }
    
    write_info("%s: response for [ip = %s port = %s id = %s]", 
	       func, ip, port, id);
  
    /* Change RAD-Identifier */
    for(p = out; p != NULL; p = p->next) {		
	if (!strcmp(p->name, "RAD-Authenticator")) {
	    continue;
	}
	*str = '\0';
	if (strcmp(p->space, "")) {
	    strncat(str, p->space, strlen(p->space) + 1);
	    strcat(str, ":");	    	   
	}	
	if (strcmp(p->vendor, "")) {
	    strncat(str, p->vendor, strlen(p->vendor));
	    strcat(str, ":");
	}	
	strncat(str, p->name, strlen(p->name));
	strcat(str, "=");
	if (strcmp(p->type, "")) {
	    strncat(str, p->type, strlen(p->type));
	    strcat(str, ":");
	}	
	if (!strcmp(p->name, "RAD-Identifier")) {
	    strncat(str, id, strlen(id));	    	    
	} else {
	    strncat(str, p->value, strlen(p->value));
	}		
	write_debug("%s: %s", func, str);
	strcat(str, "\n");
	strncat(packet, str, strlen(str));
    }    
  
    strcat(packet, "\n");
  
    /* Copy packet to buffer for openradius */
    strcpy(buf, packet);
  
    /* Free packet */
    free(packet);
}

/*
Checking triplet
*/

int check_triplet(char *ip, char *port, char *id) {
    static const char *func = "check_triplet";

    if (*ip && *port && *id) {
      return 0;
    } else {
	write_error("%s: either of %s%s%sis not defined", func, 
                    *ip ? "" : "ip ", *port ? "" : "port ", *id ? "" : "id ");
	return -1;
    }
}

/*
Initialize params
*/
void init_attr(attr* p) {
    *(p->space) = *(p->vendor) = *(p->name) \
	= *(p->type) = *(p->value) = '\0';    
}

/*
Destroy list of attributes
*/
void destroy_attr(attr* p) {
    attr *q, *l;
    q = p; 
    while(q != NULL) {
	l = q->next; 
	free(q);
	q = l;
    }
}

/*
Output data from radclient
*/
int process_radclient(char *buf, int slot_id) {
    attr *out, *p, *f;
    int num;
    char in_key[26];
    char out_key[26];
    char ip[16];
    char port[6];
    char id[4];        
    int i;
    int rc;
    second_key *tmp;
    static const char *func = "process_radclient";
    
    *in_key = *out_key = '\0';
    
    write_debug("%s: process radclient", func);

    /* Init triplet */
    
    out = (attr*)malloc(sizeof(attr));    
    out->next = NULL;
    init_attr(out);
    
    /* Init output in yacc/flex */
    init_output(out, func);    
    
    /* Start scanning of input buffer */
    start_scan(buf);       
    
    if (syntax_error) {	
	write_error("%s: packet: %s", func, buf);
	stat.dropped++;
	/* Free */
	destroy_attr(out);
	return -1;
    }

    f = out;
    out = out->next;
    free(f);

    /* Print packet */
    print_packet(out);  

    /* Create key */
    rc = create_out_key(out_key, out, slot_id);
    
    if (!rc) {
	/* Using key */    
	if (get_record_second(out_key) != NULL) {	
	    strcpy(in_key, get_record_second(out_key));
	    tmp = get_record_first(in_key);
	    if (tmp == NULL) {
		write_error("%s: out_key = %s in_key = %s", 
			    func, out_key, in_key);
		/* Free */
		destroy_attr(out);	
		return -1;
	    } else {
		tmp = get_record_first(in_key);
		tmp->flag = 0;
	    }
	} else {
	    /* Free */
	    destroy_attr(out);	
	    return -1;
	}
	
	/* Create output packet */
	work_main(out, in_key, buf);
    } else {
	strcat(buf, "\n");
    }
        
    /* Free */    
    destroy_attr(out);

    return 0;
}

/* 
Input data from openradius
*/
int process_main(char *buf) {
    attr *out, *p, *f;
    int num;
    char in_key[26];
    char out_key[26];
    second_key *s_key, *tmp;
    int radclnt, idd;
    int i;
    char ip[16];
    char port[6];
    char id[4];
    static const char *func = "process_main";

    write_debug("%s: process main", func);
    
    s_key = (second_key*)calloc(1, sizeof(second_key));

    /* Init triplet */
    out = (attr*)malloc(sizeof(attr));
    out->next = NULL;

    init_attr(out);
        
    /* Init output in yacc/flex */
    init_output(out, func);   
    
    /* Start scanning of input buffer */
    start_scan(buf);     

    if (syntax_error) {
	write_error("%s: packet: %s", func, buf);
	stat.dropped++;
	/* Free */
	free(s_key);
	destroy_attr(out);
	return -1;
    }

    f = out;
    out = out->next;
    free(f);

    /* Print packet */
    print_packet(out);  

    /* Create key */    
    *ip = *port = *id ='\0';
    for(p = out; p != NULL; p = p->next) {
	if (!strcmp(p->name, "NAS-IP-Address")) {
	    strncpy(ip, p->value, sizeof(ip));
	}
	if (!strcmp(p->name, "NAS-Port")) {
	    strncpy(port, p->value, sizeof(port));
	}
	if (!strcmp(p->name, "RAD-Identifier")) {
	    strncpy(id, p->value, sizeof(id));
	}
    }
    
    /* Check ip, port, id */    
    if (check_triplet(ip, port, id)) {
	/* Free */
	free(s_key);
	destroy_attr(out);
	return -1;
    }
    
    *in_key = '\0';
    sprintf(in_key, "%s_%s_%s", ip, port, id);
    
    /* Using key */
    if (get_record_first(in_key) != NULL) {
	/* the same key already exists */
	tmp = get_record_first(in_key);	
	write_info("%s: retransmit for ip = %s port = %s id = %s", 
		   func, ip, port, id); 	
	stat.retransmit++;
	
	*out_key = '\0';
	sprintf(out_key, "%d_%d", tmp->radclnt, tmp->id);
	radclnt = tmp->radclnt;
	idd = tmp->id;
	tmp->time = global_timestamp;	
	tmp->flag = 1;
    } else {
	/* Find out_key: radclient - id */		
	
	radclnt = cid%num_rclnt + 1;
	idd = cid/num_rclnt;
	s_key->radclnt = radclnt;
	s_key->id = idd;
	s_key->time = global_timestamp;	
	s_key->flag = 1;	

	write_info("%s: request for [ip = %s port = %s id = %s]", 
		   func, ip, port, id); 
	stat.request++;

	*out_key = '\0';
	sprintf(out_key, "%d_%d", radclnt, idd);	
	
	if (cid == 255 * num_rclnt) {
	    cid = 0;
	} else {
	    cid++;
	}	
	reg_record_first(in_key, s_key);		
	reg_record_second(out_key, in_key);
    }    

    /* Start radclient (out_key) with packet (output) */
    work_client(out, out_key, buf, ip, port, id); 

    /* Free */
    free(s_key);
    destroy_attr(out);
    
    return radclnt;
}

int processor(io_slot_t slots[], int slot_id, int io_flags) {
    int n;
    io_slot_t *ios = slots + slot_id;
    io_stream_t *out = &ios->stream.byname.out;
    io_stream_t *err = &ios->stream.byname.err;
    char msg_err[1024];
    char *p;
    int i;
    int flag;
    int cut;
    int rc;    
    static const char *func = "processor";

    write_debug("%s: process buffer_id = %d", func, slot_id);
    
    *msg_err = '\0';
	    
    if (io_flags & IO_ERROR) {
	if (io_flags & IO_READ_STDERR) {
	    *(err->last) = '\0';
	    if (strstr(err->buffer, "No free request slot - aborting query!")) { 
		if (stat.not_response == 256) {
		    write_error("%s: Overflow queue of slots!", func);
		}
	    } else {
		write_error("%s: IO_READ_STDERR [%s]", 
			    func, err->buffer);	
	    }
	} else {
	    io_flags_to_streing(msg_err, io_flags);
	    *(err->last) = '\0';
	    write_error("%s: IOERR! %s [%s]", 
			func, msg_err, err->buffer);	
	    exit(1);
	}
    }

    n = out->last - out->buffer;	
    strncat(out->accum, out->buffer, n);		

    /* Copy accumulator */
    p = (char*)malloc(PACKAGE_SIZE * sizeof(char));
    *p = '\0';
    if (!p) {
	write_error("%s: malloc", func);	
    }

    while(1) {	
	strcpy(p, out->accum);
	write_debug("buffer: %s", out->accum);
	
	/* Find \n\n */		
	flag = 0;
	for(i = 0; i < strlen(p); i++) {
	    if (p[i - 1] == '\n' && p[i] == '\n') {
		cut = i;
		flag = 1;		
		break;
	    }
	}

	if (flag) {
	    p[cut] = '\0';
	    if (strlen(p) != strlen(out->accum) - 1) {
		strcpy(out->accum, p + cut + 1);
	    } else {
		*(out->accum) = '\0';		
	    }
	    write_debug("packet: %s", p);
	    
	    if (slot_id == 0) {	
		uint64_t stats_start_ms = stats_c_now_ms();
		rc = process_main(p);	    		
		stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
		if (rc != -1) {
		    n = strlen(p);	    	
		    io_write(rc, p, n);
		    write_debug("%s: SUPER->slave", func);		   
		}
	    } else {
		rc = process_radclient(p, slot_id);	       
		if (rc != -1) {
		    n = strlen(p);
		    io_write(0, p, n);
		    stat.response++;
		    write_debug("%s: SUPER<-slave", func);		   
		}
	    }
	} else {
	    break;
	}
    }
    /* Free copy */
    free(p);

    out->last = out->buffer;
    
    return 0;
}

/* Create client */
void init_client(int slot_id) {
    int res;
    slave_process_t h;    

    res = slave(&h, args[0], args);
    
    /* For each slot_id */
    io_set_slot(slot_id, h.pid, h.in, h.out, h.err, processor);
}

/* Parse prog - path to radclient with options */
void parse_prog(void) {
    int i, ii, jj;    
    static const char *func = "parse_prog";

    args = (char**)malloc(MAX_OPT_RCLNT * sizeof(char*));
    for(i = 0; i < MAX_OPT_RCLNT; i++) {
	args[i] = (char*)malloc(MAX_OPT_SIZE * sizeof(char));
    }
    
    ii = 0;
    jj = 0;
    strcat(prog, " ");
    for(i = 0; i < strlen(prog); i++) {
	if (prog[i] == ' ') {
	    args[ii][jj] = '\0';
	    ii++;
	    jj = 0;
	} else {
	    args[ii][jj] = prog[i];
	    jj++;	    
	}
    }
    args[ii] = NULL;
}

int main(int argc, char* argv[]) {
    int rc, i;
    static const char *func = "main";
 
    *package_name = *package_version = *package_bugreport = '\0';

#ifdef PACKAGE_NAME    
    strcpy(package_name, PACKAGE_NAME);
#else
    strcpy(package_name, 
	    "Package name is not defined");
#endif
#ifdef PACKAGE_VERSION    
    strcpy(package_version, PACKAGE_VERSION);
#else
    strcpy(package_version, 
	    "Package version is not defined");
#endif
#ifdef PACKAGE_BUGREPORT
    strcpy(package_bugreport, PACKAGE_BUGREPORT);
#else
    strcpy(package_bugreport, 
	   "Package bugreport is not defined");
#endif

    if ((rc = process_args(argc, argv))) {
	exit(0);
    }

    
    /* Init sequence for change id */
    cid = 0;
        
    logging_init();    
    
    write_stat("%s: starting", func);    
    stats_c_init(&stats, "multi_radclient", stats_file, stats_interval_min);
    
    /* Create tables */    
    init_table_first();
    init_table_second();
       
    /* Init handler for clean table*/
    io_init(clean_table);
    
    /* slot 0: stdin, -out, -err */
    io_set_slot(0, 0, 1, 0, -1, processor); 

    /* Init radclient */
    for(i = 1; i <= num_rclnt; i++) {
	init_client(i);
    }
    
    /* Free args */
    for(i = 0; i < MAX_OPT_RCLNT; i++) {
	free(args[i]);
    }
    free(args);

    /* Main loop start */
    dispatch_forever();

    write_stat("%s: finished", func);    

    return 0;
}
