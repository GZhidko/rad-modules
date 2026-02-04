#ifndef MRADCLIENT_CHILD_ERR_PROXY_H
#define MRADCLIENT_CHILD_ERR_PROXY_H

#include "abstract_data_acceptor.h"
#include <unistd.h>
#include <sys/types.h>

class Logger;

class ChildErrorProxy : public AbstractDataAcceptor {
private:
    int child_id;
    pid_t child_pid;
    Logger * logger;
public:
    ChildErrorProxy();
    ChildErrorProxy(const ChildErrorProxy &);             // IMPLEMENT IT!!
    ChildErrorProxy & operator=(const ChildErrorProxy &); // IMPLEMENT IT!!
    void accept_data(time_t const, char const * const, int const);
    void setup(Logger *, int, pid_t);
};

#endif // MRADCLIENT_CHILD_ERR_PROXY_H
