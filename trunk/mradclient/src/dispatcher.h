#ifndef MRADCLIENT_DISPATCHER_H
#define MRADCLIENT_DISPATCHER_H

#include <map>

#include <iostream>

#include "logger.h"

class AbstractDataAcceptor;
class OutputBuffer;
class Balancer;

class Dispatcher {

friend std::ostream & operator<<(std::ostream & out, const Dispatcher & object);

private:
    std::map <int, AbstractDataAcceptor *> input_buffers;
    std::map <int, OutputBuffer *> output_buffers;
    Logger * logger;
    Balancer * balancer;
public:
    Dispatcher(Logger *, Balancer *);

    Dispatcher(const Dispatcher &);             // IMPLEMENT IT!!
    Dispatcher & operator=(const Dispatcher &); // IMPLEMENT IT!!

    void registre(int, AbstractDataAcceptor *);
    void registre(int, OutputBuffer *);
    int dispatch();
    void vanish();
};

#endif // MRADCLIENT_DISPATCHER_H
