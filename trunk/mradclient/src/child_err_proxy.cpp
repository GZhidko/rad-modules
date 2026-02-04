#include "child_err_proxy.h"
#include "logger.h"

ChildErrorProxy::ChildErrorProxy() :
    child_id(0),
    child_pid(0),
    logger(0)
{
}

void
ChildErrorProxy::accept_data(time_t const now, char const * const data, int const len) {
    *logger << loglevel<debug>
            << "STDERR Child " << child_id
            << ", PID=" << child_pid
            << endl
            << loglevel<warning>
            << raw_mode
            << std::string(data, len)
            << common_mode
            << loglevel<debug>
            << "STDERR Child " << child_id;
}

void
ChildErrorProxy::setup(Logger * l, int cid, pid_t cpid) {
    child_id = cid;
    child_pid = cpid;
    logger = l;
}
