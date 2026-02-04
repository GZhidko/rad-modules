#ifndef MRADCLIENT_CHILD_PROXY_H
#define MRADCLIENT_CHILD_PROXY_H

#include "abstract_data_acceptor.h"

class Balancer;

class ChildProxy : public AbstractDataAcceptor {
private:
    int child_id;
    class Balancer * balancer;
public:
    ChildProxy();
    ChildProxy(Balancer *, int child_id);
    ChildProxy(const ChildProxy &);             // IMPLEMENT IT!!
    ChildProxy & operator=(const ChildProxy &); // IMPLEMENT IT!!
    void accept_data(time_t const, char const * const, int const);
    void setup(Balancer *, int child_id);
};

#endif // MRADCLIENT_CHILD_PROXY_H
