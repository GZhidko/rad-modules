#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "log.h"
#include "radre.h"

const char *stats_file = NULL;
int stats_interval_min = 5;

char ** parse_args(int argc, char *argv[]) {
  int ch;
  setup_logger(argv[0]);
  int n = 1;
  while ((ch = getopt(argc, argv, "hds:i:")) != -1) {
    n++;
    switch (ch) {
    case 'h':
      printf("Version: " VERSION "\n"
             "Usage: %s -[hd] [-s statfile] [-i minutes] file [file [...]]\n"
             "\t-h -- help and exit\n"
             "\t-d -- debugging on\n"
             , argv[0]);
      exit(0);
      break;
    case 'd':
      set_debug(1);
      break;
    case 's':
      stats_file = optarg;
      break;
    case 'i':
      stats_interval_min = atoi(optarg);
      break;
    }
  }
  return argv + n;
}
