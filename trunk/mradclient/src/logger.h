/****************************************************

Объект протоколирования имеет интерфейс принимающего буфера.
Это позволяет присоединить его непосредственно к stderr
дочерних процессов.

формат:
data mradclient[PID] message

*****************************************************/

#ifndef MRADCLIENT_LOGGER_H
#define MRADCLIENT_LOGGER_H

#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include "output_buffer.h"
#include "abstract_data_acceptor.h"

enum LogLevel {
        fatal = 100,   // аварийное завершение
        error = 80,    // ошибки, приводящие к завершению процессов, которые не должны завершаться
        warning = 60,  // ошибки, нарушающие транспорт отдельных запросов
        info = 40,     // периодическая сводная информация (уровень пригодный для продакшена)
        rinfo = 30,    // reach info
        debug = 20     // полная отладка
};

class Logger : public AbstractDataAcceptor {

template<enum LogLevel E> friend Logger & loglevel(Logger &);
friend Logger & raw_mode(Logger &);
friend Logger & common_mode(Logger &);

private:
    OutputBuffer * data_outer;
    bool print_prefix;
    time_t last_time;
    char prefix[42];   // ,-)
    size_t prefix_len;
    enum LogLevel log_level;
    enum LogLevel current_log_level;
    bool raw_mode;

public:
    Logger(OutputBuffer *, LogLevel);

    Logger(const Logger &);             // IMPLEMENT IT!!
    Logger & operator=(const Logger &); // IMPLEMENT IT!!

    // может вообще перенести в приват? очень низкоуровнево
    void write(char const * const data, int const len);

    // интерфейс дата-акцептора
    void accept_data(time_t const, char const * const, int const);

    // С-printf интерфейс (надо бы изжить)
    void log(LogLevel, time_t const now, const char * format, ...);

    // надо отказываться от практики передачи времени
    // при каждом вызове;
    // правильный путь -- устанавливать время после select
    // и дальше использовать его
    void update_time(time_t const now);

    // С++ << интерфейс
    Logger & operator<<(char const * c);
    Logger & operator<<(int n);
    Logger & operator<<(std::string const & s) { *this << s.c_str(); return *this;} // XXXX
    // аппликатор для безаргументных манипуляторов установки уровня отладки
    Logger & operator<<(Logger & (*pf)(Logger &));
};

template<enum LogLevel E> Logger & loglevel(Logger & l);
Logger & endl(Logger &);
Logger & raw_mode(Logger &);
Logger & common_mode(Logger &);

#endif // MRADCLIENT_LOGGER_H
