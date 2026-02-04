#ifndef MRADCLIENT_BALANCER_H
#define MRADCLIENT_BALANCER_H

#include "abstract_data_acceptor.h"
#include "heaver.h"
#include "../../stats_c.h"

class ChildProxy;
class ChildErrorProxy;
class Logger;
class Configuration;
class OutputBuffer;
class InputBuffer;
class Dispatcher;

class Balancer : public AbstractDataAcceptor {

private:
    Configuration * configuration;
    Logger * logger;
    OutputBuffer * output_buffer; // буфер из которого будем отвечать радиусу
    int size; // пока размер пула не меняется динамически, но скорее всего, -- будет
    OutputBuffer * child_in_buffers; // это буферы, через которые будем говорить клиентам
    ChildProxy * child_input_proxies;
    InputBuffer * child_out_buffers;
    ChildErrorProxy * child_error_proxies;
    Heaver heaver;
    class Stat {
    public:
        // переданные пакеты
        int from_master; // от радиуса
        int to_master;   // к радиусу
        int from_child;  // от клиента
        int to_child;    // к клиенту
        // ошибки
        // -- не распарсился пакет от радиуса
        int error_master_pkg;
        // -- не хватило свободных клиентов
        int error_can_not_find_child;
        // -- ответ от клиента не распарсился
        int error_client_pkg;
        // -- нет информации к ответу клиента (это не фатально)
        int warning_vanish_client;
        // счётчики
        int erase_count;
        int retry_count;
        time_t last_print;
        Stat();
        void drop(time_t);
    } stat;
    stats_c_t stats;

public:
    Balancer(Configuration *, Logger *, OutputBuffer *);
    ~Balancer();

    Balancer(const Balancer &);             // IMPLEMENT IT!!
    Balancer & operator=(const Balancer &); // IMPLEMENT IT!!

    // это можно было бы разместить в конструкторе, по логике,
    // но уж очень тяжела операция... а вдруг исключение?
    void create_and_bind_childs(Dispatcher * dispatcher);

    // интерфейс акцептора
    void accept_data(time_t const, char const * const, int const);

    // интерфейс для получения данных от потомков
    void accept_data_from_child(int child_id, time_t read_time, char const * data, int len);

    // интерфейс для Heaver
    void send_data_to_master(time_t now, char const * data, int len);
    void send_data_to_child(int cid, time_t now, char const * data, int len);
    int pool_size() const; // пока мы можем просто узнать, сколько есть потомков,
                           // но именно в этом месте следует развиваться для
                           // динамического изменения числа потомков

    // интерфейс обработки времени для диспетчера
    void time(time_t now);
};

#endif // MRADCLIENT_BALANCER_H
