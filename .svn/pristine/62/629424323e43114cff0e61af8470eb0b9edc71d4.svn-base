#ifndef MRADCLIENT_HEAVER_CLUSTER_H
#define MRADCLIENT_HEAVER_CLUSTER_H

#include <map>
#include <string>
#include <time.h>
#include <iostream>

class Logger;
class HeaverCluster;

class HeaverCluster {
//template<class S> friend S & operator<<(S & out, const HeaverCluster & object);
friend Logger & operator<<(Logger & out, const HeaverCluster & object);
private:
    class pkg_record_t { // здесь же можно хранить очереди, если и когда они понадобятся
    public:
        std::string authenticator;
        int child_id;
        time_t stamp;
        pkg_record_t() : authenticator(), child_id(0), stamp(0) {} // stop warnings
    };
    std::map<std::string, pkg_record_t*> authenticators; // Authenticator -> record*
    std::map<int, pkg_record_t*> child_ids;              // child_id -> record
    int finger;
    time_t last_clean;
public:
    HeaverCluster();
    ~HeaverCluster();
    int get_child_id_and_update_record(std::string const & authenticator, time_t now, int pool_size, int time_out, int & count_erase, bool & is_retry);
    int vanish(int child_id);
};

/*
template<class S>
S &
operator<< (S & l, HeaverCluster const & o) {
//Logger &
//operator<<(Logger & l, const HeaverCluster & o) {
    l << "  child_ids:\n";
    for(std::map<int, HeaverCluster::pkg_record_t>::const_iterator i(o.child_ids.begin());
        i != o.child_ids.end();
        ++i) {
        l << "    " << (*i).first << ": " <<
                       (*i).second.child_id << " " <<
                       (*i).second.stamp << " " <<
                       (*i).second.authenticator.c_str() << "\n";
    }
    l << "  authenticators:\n";
    for(std::map<std::string, HeaverCluster::pkg_record_t>::const_iterator i(o.authenticators.begin());
        i != o.authenticators.end();
        ++i) {
        l << "    " << (*i).first.c_str() << ": " <<
                       (*i).second.child_id << " " <<
                       (*i).second.stamp << " " <<
                       (*i).second.authenticator.c_str() << "\n";
    }
    l << "  finger = " << o.finger << "\n"
         "  .\n";
    return l;
}
*/


#endif // MRADCLIENT_HEAVER_CLUSTER_H
