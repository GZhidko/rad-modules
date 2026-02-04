#include "configuration.h"
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

const char * const Configuration::help_message =
"Usage:\n"
"mradclient [-r] [-u retry-ind-attr] [-c number] [-t timeout] [-d debuglevel] -- /path/to/radclient -oprionts -of -radclient\n"
"mradclient -h\n"
"mradclient -V\n"
"\n"
"options:\n"
" -r                -- remove retry-ind-attr; don't transmit it to radclient\n"
" -u retry-ind-attr -- spec. attr. to detect tetrys\n"
"                      (def. 'GLDN-MRadClient-Unifier')\n"
" -c NN             -- number of childs (def. 2)\n"
" -t NN             -- timeout\n"
" -d debuglevel     -- debug level (def. 'info')\n"
" -s statfile       -- JSON stats output file\n"
" -i minutes        -- stats interval minutes (def. 5)\n"
"debug levels:\n"
"  fatal\n"
"  error\n"
"  warning\n"
"  rich_info\n"
"  info (default)\n"
"  debug\n"
"\n";

const char * const Configuration::version_message =
"Version "
PACKAGE_VERSION
" (Alphard)\n";

Configuration::Configuration() :
    child_argv(0),
    child_num(2),
    version_flag(false),
    help_flag(false),
    time_out(300),
    debug_level(info),
    incorrerc_log_level(false),
    uniq_attr_name("GLDN-MRadClient-Unifier"),
    remove_uniq_attr_flag(false),
    stats_file(""),
    stats_interval_min(5)
{
}

Configuration::~Configuration() {
    if (child_argv) {
        for (char **c(child_argv); *c; ++c) {
            free(*c);
        }
        free(child_argv);
    }
}

void
Configuration::load_opts(int argc, char * const argv[]) {
    for(bool cont(true); cont;) {
        int o(getopt(argc, argv, "u:c:C:t:d:s:i:Vhr"));
        switch(o) {
            case -1:
                cont = false;
                break;
            case 'c':
                child_num = -1;
                sscanf(optarg, "%d", &child_num);
                break;
            case 't':
                time_out = -1;
                sscanf(optarg, "%d", &time_out);
                break;
            case 'd':
                if      (strcmp(optarg, "fatal"    ) == 0) { debug_level = fatal; }
                else if (strcmp(optarg, "error"    ) == 0) { debug_level = error; }
                else if (strcmp(optarg, "warning"  ) == 0) { debug_level = warning; }
                else if (strcmp(optarg, "rich_info") == 0) { debug_level = rinfo; }
                else if (strcmp(optarg, "info"     ) == 0) { debug_level = info; }
                else if (strcmp(optarg, "debug"    ) == 0) { debug_level = debug; }
                else {
                    incorrerc_log_level = true;
                }
                break;
            case 'V':
                version_flag = true;
                break;
            case 'h':
                help_flag = true;
                break;
            case 'u':
                uniq_attr_name = optarg;
                break;
            case 'r':
                remove_uniq_attr_flag = true;
                break;
            case 's':
                stats_file = optarg;
                break;
            case 'i':
                stats_interval_min = atoi(optarg);
                break;
            default:
                cont = false;
        }
    }
    int n(0);
    for (int i(optind); i < argc; ++i) {
        char * s(argv[i]);
        if (s[0] == '-' && s[1] == '-' && s[2] == 0) { // left '--'
            continue;
        }
        if (child_argv == 0) {
            child_argv = (char **)calloc(argc - i + 1, sizeof(*child_argv));
        }
        int l(strlen(s));
        child_argv[n] = (char *)malloc(l+1);
        strcpy(child_argv[n], s);
        ++n;
    }
    if (n) {
        child_argv[n] = NULL;
    }
}

int
Configuration::validate() const {
    if (child_num <= 0) return 1;
    if (time_out <= 0) return 2;
    if (child_argv == 0) return 3;
    if (incorrerc_log_level) return 4;
    if (uniq_attr_name.empty()) return 5;
    return 0;
}

bool
Configuration::print_help() const {
    return help_flag;
}

bool
Configuration::print_version() const {
    return version_flag;
}

int
Configuration::get_child_num() const {
    return child_num;
}

int
Configuration::get_timeout() const {
    return time_out;
}

LogLevel
Configuration::get_loglevel() const {
    return debug_level;
}

char * const * const
Configuration::get_child_argv() const {
    return child_argv;
}

std::string const &
Configuration::get_uniq_attr_name() const {
    return uniq_attr_name;
}

bool
Configuration::get_remove_uniq_attr_flag() const {
    return remove_uniq_attr_flag;
}

std::string const &
Configuration::get_stats_file() const {
    return stats_file;
}

int
Configuration::get_stats_interval_min() const {
    return stats_interval_min;
}

std::ostream & operator<<(std::ostream & out, const Configuration & o) {
    out << "Configuration: child_argv =";
    if (o.child_argv) {
        for (char **c(o.child_argv); *c; ++c) {
            out << " \"" << *c << "\"";
        }
    } else {
        out << " [empty]";
    }
    out << "; child_num = " << o.child_num
        << "; time_out = " << o.time_out;
    return out;
}
