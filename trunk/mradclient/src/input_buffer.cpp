#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <assert.h>
#include "input_buffer.h"

// хорошо бы избавиться от конструктора по умолчанию

InputBuffer::InputBuffer() :
    buffer_size(10),
    buffer((char *)malloc(buffer_size)),
    buffer_len(0),
    nl_count(0),
    next_data_acceptor(0)
{
}

InputBuffer::InputBuffer(AbstractDataAcceptor * acc) :
    buffer_size(10),
    buffer((char *)malloc(buffer_size)),
    buffer_len(0),
    nl_count(0),
    next_data_acceptor(acc)
{
}

void
InputBuffer::setup(AbstractDataAcceptor * acc) {
    next_data_acceptor = acc;
}

void
InputBuffer::accept_data(time_t const read_time, char const * const data, int const len) {
//    std::cerr << "InputBuffer::accept_data() " << this << " len=" << len << "\n";
    if (buffer_size < buffer_len + len) {
        size_t new_size(buffer_len + len);
        assert(new_size < 102400);
        char * b((char *)malloc(new_size));
        memcpy(b, buffer, buffer_len);
        free(buffer);
        buffer = b;
        buffer_size = new_size;
    }
    char const * end(data + len);
    char * t(buffer + buffer_len);
    for (char const * i(data); i != end; ++i) {
        *t = *i;
        ++buffer_len;
        ++t;
        if (*i == '\n') {
            ++nl_count;
            if (nl_count > 1) {
                next_data_acceptor->accept_data(read_time, buffer, buffer_len);
                t = buffer;
                buffer_len = 0;
            }
        } else {
            nl_count = 0;
        }
    }
}

InputBuffer::~InputBuffer() {
    free(buffer);
}
