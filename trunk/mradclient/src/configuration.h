#ifndef MRADCLIENT_CONFIGURATION_H
#define MRADCLIENT_CONFIGURATION_H

#include <iostream>
#include <string>
#include "logger.h"

class Configuration {

friend std::ostream & operator<<(std::ostream & out, const Configuration & object);

private:
    char ** child_argv;
    int child_num;
    bool version_flag;
    bool help_flag;
    int time_out;
    LogLevel debug_level;
    bool incorrerc_log_level;
    std::string uniq_attr_name;
    bool remove_uniq_attr_flag;
    std::string stats_file;
    int stats_interval_min;

public:
    static const char * const help_message;
    static const char * const version_message;

    Configuration();
    ~Configuration();
    Configuration(const Configuration &);             // IMPLEMENT IT!!
    Configuration & operator=(const Configuration &); // IMPLEMENT IT!!
    void load_opts(int argc, char * const argv[]);
    int validate() const;
    bool print_version() const;
    bool print_help() const;
    int get_child_num() const;
    int get_timeout() const;
    LogLevel get_loglevel() const;
    char * const * const get_child_argv() const;
    std::string const & get_uniq_attr_name() const;
    bool get_remove_uniq_attr_flag() const;
    std::string const & get_stats_file() const;
    int get_stats_interval_min() const;
};

#endif // MRADCLIENT_CONFIGURATION_H
