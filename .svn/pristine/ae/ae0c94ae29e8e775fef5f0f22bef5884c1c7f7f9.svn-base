/*********************************************************

Это машина состояний, которая накапливает данные до тех
пор, пока не наберёт полный пакет. Данные всегда поглощаются
полностью. Собранный пакет передаётся дальше.

**********************************************************/

#ifndef MRADCLIENT_INPUT_BUFFER_H
#define MRADCLIENT_INPUT_BUFFER_H

#include "abstract_data_acceptor.h"

class InputBuffer : public AbstractDataAcceptor {
private:
    unsigned int buffer_size;
    char *buffer;
    unsigned int buffer_len;
    unsigned int nl_count;
    AbstractDataAcceptor * next_data_acceptor;
public:
    InputBuffer();
    InputBuffer(AbstractDataAcceptor *);
    void setup(AbstractDataAcceptor *);
    InputBuffer(const InputBuffer &);             // IMPLEMENT IT!!
    InputBuffer & operator=(const InputBuffer &); // IMPLEMENT IT!!
    void accept_data(time_t const, char const * const, int const);
    ~InputBuffer();
};

#endif // MRADCLIENT_INPUT_BUFFER_H
