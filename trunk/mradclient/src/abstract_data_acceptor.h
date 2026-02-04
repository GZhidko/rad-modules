#ifndef MRADCLIENT_ABSTRACT_DATA_ACCEPTOR_H
#define MRADCLIENT_ABSTRACT_DATA_ACCEPTOR_H

#include <time.h>

class AbstractDataAcceptor {
private:
public:
    virtual void accept_data(time_t const t, char const * const data, int const len) = 0;
//
// виртуальный конструктор надо будет добавить сразу же, как появится
// действительно настоящее наследование; но появится ли оно?
//
//  virtual ~AbstractDataAcceptor() {};
};

#endif // MRADCLIENT_ABSTRACT_DATA_ACCEPTOR_H
