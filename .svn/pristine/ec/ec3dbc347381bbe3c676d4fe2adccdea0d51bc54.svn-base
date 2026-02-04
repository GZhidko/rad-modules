#include <sys/select.h>
#include "dispatcher.h"
#include "input_buffer.h"
#include "output_buffer.h"
#include "balancer.h"
#include <errno.h>
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string.h>

Dispatcher::Dispatcher(Logger * l, Balancer * b):
    input_buffers(),
    output_buffers(),
    logger(l),
    balancer(b)
{
}

void
Dispatcher::registre(int fd, AbstractDataAcceptor * o) {
    input_buffers[fd] = o;
}

void
Dispatcher::registre(int fd, OutputBuffer * o) {
    output_buffers[fd] = o;
}

int
Dispatcher::dispatch() {
    struct timeval tv;
    tv.tv_sec = 1;  // секунды
    tv.tv_usec = 0; // микросекунды
    time_t now;
    now = time(&now);
    logger->update_time(now);
    for (;;) {
        int max_fd(-1);
        fd_set read_fds, write_fds;
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);
        // read from...
        for (std::map<int, AbstractDataAcceptor *>::const_iterator
              i(input_buffers.begin());
              i != input_buffers.end();
              ++i) {
            int fd = (*i).first;
            FD_SET(fd, &read_fds);
            if (fd > max_fd) {
                max_fd = fd;
            }
        }
        // write to...
        for (std::map<int, OutputBuffer *>::const_iterator
             i(output_buffers.begin());
             i != output_buffers.end();
             ++i) {
            int len = (*i).second->data_len();
            if (len > 0) {
                int fd = (*i).first;
                FD_SET(fd, &write_fds);
                if (fd > max_fd) {
                    max_fd = fd;
                }
            }
        }
        if (max_fd == -1) {
            return 0; // мы не ждём ни ввода ни вывода; завершился цикл до-записи остатков; выходим
        }
        int res = select(max_fd+1, &read_fds, &write_fds, NULL, &tv);
        now = time(&now);
        logger->update_time(now);
        balancer->time(now);
//        *logger << loglevel<debug> << "Select... (max_fd = " << max_fd << ")" << endl;
        if (res < 0) {
            *logger << loglevel<error> << "Leave main loop: errno = " << errno << endl;
            return 2; // совсем авария?
        }
        // res == 0 -- timeout
        if (res > 0) {
            // проверяем все дескрипторы, что не есть правильно
            // что есть чересчур
            // read
            for (std::map<int, AbstractDataAcceptor *>::const_iterator
                 i(input_buffers.begin());
                 i != input_buffers.end();
                 ++i) {
                int fd = (*i).first;
                if (FD_ISSET(fd, &read_fds)) {
                    char b[4000];
                    int readed = read(fd, b, 4000);
                    if (readed == 0) {
                        *logger << loglevel<error> << "Leave main loop: Child close stream " << fd << endl;
                        return 1;
                    }
                    (*i).second->accept_data(now, b, readed);
                }
            }
            // write
            for (std::map<int, OutputBuffer *>::const_iterator
                 i(output_buffers.begin());
                 i != output_buffers.end();
                 ++i) {
                int fd = (*i).first;
                if (FD_ISSET(fd, &write_fds)) {
                    OutputBuffer * o((*i).second);
                    void * data = o->data_ptr();
                    size_t len = o->data_len();
                    size_t l = write(fd, data, len);
                    o->data_cat(l);
                }
            }
        }
    }
    return 255; // как это мы сюда попали?
}

void
Dispatcher::vanish() {
    input_buffers.clear();
    output_buffers.clear();
}

std::ostream &
operator<<(std::ostream & out, const Dispatcher & obj) {
    out << "Dispatcher:" << std::endl <<
           " input:" << std::endl;
    for (std::map<int, AbstractDataAcceptor *>::const_iterator
         i(obj.input_buffers.begin());
         i != obj.input_buffers.end();
         ++i) {
        out << "  " << (*i).first << " => " << (*i).second << std::endl;
    }
    out << " output:" << std::endl;
    for (std::map<int, OutputBuffer *>::const_iterator
         i(obj.output_buffers.begin());
         i != obj.output_buffers.end();
         ++i) {
        out << "  " << (*i).first << " => " << (*i).second << std::endl;
    }
    out << "----" << std::endl;
    return out;
}
