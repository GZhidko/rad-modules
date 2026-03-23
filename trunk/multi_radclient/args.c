#include "args.h"
#include <stdlib.h>

int process_args(int argc, char *argv[]) {
    int i, j;
    int q;
    int flag = 0;
    char num[10];
    char help_msg[] = "usage: multi_radclient [-d all|debug|info|warning|stat|error|quiet] \
[-nvh] -r path_to_radclient [-s statfile] [-i minutes]\n\n\
-d \t logging level, default: info\n\
-r \t path to radclient with options\n\t example:\
/usr/local/lib/openradius/radclient -q -c 2 -t 1 localhost:1645 h1dd3n\n\
-n \t number of radclients, default: 7\n\
-t \t expire time for request, default: 15 sec\n\
-s \t stats output file\n\
-i \t stats interval minutes (default: 5)\n\
-v \t version\n\
-h \t help\n"; 
        
    num_rclnt = 7; 
    exp_time = 15;
    stats_file = NULL;
    stats_interval_min = 5;

    *prog = '\0';	        
    args = (char**)malloc(MAX_OPT_RCLNT * sizeof(char*));
    for(i = 0; i < MAX_OPT_RCLNT; i++) {
	args[i] = (char*)malloc(MAX_OPT_SIZE * sizeof(char));
    }

    if (argc == 1) {
	printf("%s", help_msg);
	return 1;
    };    
    
    j = 0;
    for (i = 1; i < argc; i++) {	
	if (flag == 1) {
	    strcpy(args[j], argv[i]);		
	    j++;
	    continue;
	}
	if (flag == 2) {	 
	    if (!strcasecmp("ALL", argv[i])) {
		set_logging_level(ALL);
	    } else if (!strcasecmp("DEBUG", argv[i])) {
		set_logging_level(DEBUG);
	    } else if (!strcasecmp("INFO", argv[i])) {
		set_logging_level(INFO);
	    } else if (!strcasecmp("WARNING", argv[i])) {
		set_logging_level(WARNING);
	    } else if (!strcasecmp("STAT", argv[i])) {
		set_logging_level(STAT);
	    } else if (!strcasecmp("ERROR", argv[i])) {
		set_logging_level(ERROR);
	    } else if (!strcasecmp("QUIET", argv[i])) {
		set_logging_level(QUIET);	    
	    } else {
		printf("Error! Option -d must be have argument, see help.\n\n");
		printf("%s", help_msg);
		return 1;	 
	    }	    
	    flag = 0;
	}
	if (flag == 3) {	    	    
	    num_rclnt = atoi(argv[i]);
	    if (num_rclnt >= IO_SLOT_NUMBER) {
		printf("error: number of radclient should not be more %d\n", 
		       IO_SLOT_NUMBER - 1);
		return 1;
	    }
	    flag = 0;
	}
	if (flag == 4) {
	    exp_time = atoi(argv[i]);
	    flag = 0;
	}
	if (flag == 5) {
	    stats_file = argv[i];
	    flag = 0;
	}
	if (flag == 6) {
	    stats_interval_min = atoi(argv[i]);
	    flag = 0;
	}

	if (strcmp("-v", argv[i]) == 0) { 
	    printf("package: %s\nversion: %s\nauthor: %s\n",
		   package_name, package_version, package_bugreport);
	    return 1;	 
	} else if (strcmp("-r", argv[i]) == 0) {	        
	    flag = 1;	    
	} else if (strcmp("-h", argv[i]) == 0) {	        
	    printf("%s", help_msg);
	    return 1;	 
	} else if (strcmp("-d", argv[i]) == 0) {
	    flag = 2;	    
	} else if (strcmp("-n", argv[i]) == 0) {	        
	    flag = 3;	    
	} else if (strcmp("-t", argv[i]) == 0) {	        
	    flag = 4;	    
	} else if (strcmp("-s", argv[i]) == 0) {
	    flag = 5;
	} else if (strcmp("-i", argv[i]) == 0) {
	    flag = 6;
	}
    }
    args[j] = NULL;
    return 0;
}
