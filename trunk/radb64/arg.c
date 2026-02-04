#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "log.h"
#include "b64.h"
#include "loop.h"
#include "radb64.h"

const char *stats_file = NULL;
int stats_interval_min = 5;

int parse_args(int argc, char *argv[]) {
  int ch;
  setup_logger(argv[0]);
  while ((ch = getopt(argc, argv, "hDdes:i:")) != -1) {
    switch (ch) {
    case 'h':
      printf("Version: " VERSION "\n"
             "Usage: %s -[hDde] [-s statfile] [-i minutes]\n"
             "\t-h -- help and exit\n"
             "\t-D -- debugging on\n"
             "\t-d -- decode mode\n"
             "\t-e -- encode mode\n"
             , argv[0]);
      exit(0);
      break;
    case 'D':
      set_debug(1);
      break;
    case 'e':
      processor = b64_enc;
      break;
    case 'd':
      processor = b64_dec;
      break;
    case 's':
      stats_file = optarg;
      break;
    case 'i':
      stats_interval_min = atoi(optarg);
      break;
    }
  }
}
