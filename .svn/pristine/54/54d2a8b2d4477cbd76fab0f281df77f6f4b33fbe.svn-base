#include "child_proxy.h"
#include "balancer.h"

ChildProxy::ChildProxy() {
}

ChildProxy::ChildProxy(Balancer * b, int cid) :
    child_id(cid),
    balancer(b)
{
}

void
ChildProxy::accept_data(time_t const read_time, char const * const data, int const len) {
    balancer->accept_data_from_child(child_id, read_time, data, len);
}

void
ChildProxy::setup(Balancer * b, int cid) {
    child_id = cid;
    balancer = b;
}
