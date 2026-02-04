#include <iostream>

#include "output_buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

OutputBuffer::OutputBuffer() :
    buffer_size(1024),
    buffer((char *)malloc(buffer_size)),
    buffer_len(0)
{
}

OutputBuffer::~OutputBuffer() {
    free(buffer);
}

void *
OutputBuffer::data_ptr() const {
    return (void *)buffer;
}

size_t
OutputBuffer::data_len() const {
    return buffer_len;
}

void
OutputBuffer::data_cat(size_t len) {
    size_t new_len(buffer_len - len);
//    std::cerr << "OutputBuffer::data_cat() " << this << " len=" << len << " new_len=" << buffer_len << "\n";
    if (new_len > 0) {
        memmove(buffer, buffer + len, new_len);
        buffer_len = new_len;
    } else {
        buffer_len = 0;
    }
}

void
OutputBuffer::accept_data(char const * const data, size_t len) {
    if (buffer_len + len > buffer_size) {
        size_t new_size(buffer_len + len);
        if (new_size > 102400) {
        std::cerr << "===============================\n";
        for(size_t i(0); i < buffer_len; ++i) {
           std::cerr << buffer[i];
        }
        std::cerr << "===============================\n";
        }
        assert(new_size < 102400);
        char * b((char *)malloc(new_size));
        memcpy(b, buffer, buffer_len);
        free(buffer);
        buffer = b;
        buffer_size = new_size;
    }
    memcpy(buffer + buffer_len, data, len);
    buffer_len += len;
//    std::cerr << "OutputBuffer::accept_data() " << this << " len=" << len << " new_len=" << buffer_len << "\n";
}
