#include "balancer.h"
#include "child_proxy.h"
#include "logger.h"
#include "configuration.h"
#include "output_buffer.h"
#include "dispatcher.h"
#include "input_buffer.h"
#include "child_err_proxy.h"

#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>

// ------------------------------------------------------
// UTIL
// ------------------------------------------------------

void raise_child(char * const * const argv, pid_t * p, int * in, int * out, int * err) {
    pid_t pid;
    int ch_in[2];
    int ch_out[2];
    int ch_err[2];
    int *chanel[] = {ch_in, ch_out, ch_err};
    int i;
    int r;
    // создаём три пайпа
    for (i=0; i<3; i++) {
        r = pipe(chanel[i]);
        assert(r >= 0);
    }
    pid = fork();
    assert(pid >= 0);
    if (pid == 0) { // это мы уже в дочернем процессе
        close(ch_in[1]);
        close(ch_out[0]);
        close(ch_err[0]);
        dup2(ch_in[0], 0);
        dup2(ch_out[1], 1);
        dup2(ch_err[1], 2);
        int res = execv(argv[0], argv);
        // здесь мы не должны оказаться
        assert(1);
        printf("ok. 8-O res=%d errno=%d\n", res, errno);
        close(ch_in[0]);
        close(ch_out[1]);
        close(ch_err[1]);
        exit(1);
    } else { // а эта ветка наша
        close(ch_in[0]);
        close(ch_out[1]);
        close(ch_err[1]);
        *p = pid;
        *in = ch_in[1];
        *out = ch_out[0];
        *err = ch_err[0];
    }
}

// ------------------------------------------------------

Balancer::Balancer(Configuration * c, Logger * l, OutputBuffer * o) :
    configuration(c),
    logger(l),
    output_buffer(o),
    size(configuration->get_child_num()),
    child_in_buffers(new OutputBuffer[size]),
    child_input_proxies(new ChildProxy[size]),
    child_out_buffers(new InputBuffer[size]),
    child_error_proxies(new ChildErrorProxy[size]),
    heaver(this, logger, c),
    stat()
{
    const char *file = configuration->get_stats_file().empty() ? NULL : configuration->get_stats_file().c_str();
    stats_c_init(&stats, "mradclient", file, configuration->get_stats_interval_min());
}

Balancer::~Balancer() {
    delete[] child_in_buffers;
    delete[] child_input_proxies;
    delete[] child_out_buffers;
    delete[] child_error_proxies;
}

void
Balancer::create_and_bind_childs(Dispatcher * dispatcher) {
    for (int i(0); i < size; ++i) {
        int in, out, err;
        pid_t pid;
        raise_child(configuration->get_child_argv(), &pid, &in, &out, &err);
        dispatcher->registre(in, child_in_buffers + i); // т.е. &child_out_buffers[i]
        // из диспетчера идёт в буфер, из буфера в прокси из прокси в балансер
        dispatcher->registre(out, child_out_buffers + i);
        child_out_buffers[i].setup(child_input_proxies + i);
        child_input_proxies[i].setup(this, i);
        // из диспетчера прям в логер (хорошо бы проксю что-ли тоже сделать)
        dispatcher->registre(err, child_error_proxies + i);
        child_error_proxies[i].setup(logger, i, pid);
    }
}

// ------------------------------------------------------

void
Balancer::accept_data(time_t const now, char const * const data, int const len) {
    uint64_t stats_start_ms = stats_c_now_ms();
    ++stat.from_master;
    // здесь балансировщик разбирает пакет и принимает решение
    //logger->accept_data(now, "---\n", 4);
    //logger->accept_data(now, data, len);
    //logger->accept_data(now, "--.\n", 4);
    int count_erase;
    bool is_retry;
    HeaverError e = heaver.send_to_slave(now, data, len, count_erase, is_retry);
    if (is_retry) {
        ++stat.retry_count;
    }
    stat.erase_count += count_erase;
    if (e == parser_error) {
        ++stat.error_master_pkg;
    }
    if (e == can_not_find_child) {
        ++stat.error_can_not_find_child;
    }
    stats_c_record(&stats, stats_c_now_ms() - stats_start_ms);
}

void
Balancer::accept_data_from_child(int child_id, time_t now, char const * data, int len) {
    ++stat.from_child;
    // здесь балансировщик принимает данные от клиента
    //logger->accept_data(now, "--- CLIENT\n", 11);
    //logger->accept_data(now, data, len);
    //logger->accept_data(now, "--.\n", 4);
    HeaverError e = heaver.send_to_master(child_id, now, data, len);
    if (e == warning_vanish_client) {
        ++stat.warning_vanish_client;
    }
    if (e == parser_error) {
//        *logger << loglevel<error> << "Balancer::accept_data_from_child ++stat.error_client_pkg\n";
        ++stat.error_client_pkg;
    }
}

// ------------------------------------------------------

void
Balancer::time(time_t now) {
    if (now - stat.last_print >= 10) {
        *logger << loglevel<info> << "Stat: " <<
                   "C->" << stat.from_master <<
                   "->me->" << stat.to_child <<
                   "->R->" << stat.from_child <<
                   "->me->" << stat.to_master <<
                   "->C"
                   " retury=" << stat.retry_count <<
                   " timeout=" << stat.erase_count <<
                   " ERRs:" <<
                   " pkg_from_R=" << stat.error_master_pkg <<
                   " can_not_find_C=" << stat.error_can_not_find_child <<
                   " pkg_from_C=" << stat.error_client_pkg <<
                   " what_C=" << stat.warning_vanish_client <<
                   endl;
        stat.drop(now);
    }
}

// ------------------------------------------------------


// time_t now уже не используется, это навсегда?
void
Balancer::send_data_to_master(time_t now, char const * data, int len) {
    ++stat.to_master;
    *logger << loglevel<debug> << "Balancer::send_data_to_master stat.to_master=" << stat.to_master
            << endl
            << std::string(data, len)
            << ".\n";
    output_buffer->accept_data(data, len);
}

// time_t now уже не используется, это навсегда?
void
Balancer::send_data_to_child(int cid, time_t now, char const * data, int len) {
    ++stat.to_child;
    child_in_buffers[cid].accept_data(data, len);
}

int
Balancer::pool_size() const {
    return size;
}

// ------------------------------------------------------

Balancer::Stat::Stat() :
    from_master(0),
    to_master(0),
    from_child(0),
    to_child(0),
    error_master_pkg(0),
    error_can_not_find_child(0),
    error_client_pkg(0),
    warning_vanish_client(0),
    erase_count(0),
    retry_count(0),
    last_print(0)
{
}

void
Balancer::Stat::drop(time_t t) {
    from_master = 0;
    to_master = 0;
    from_child = 0;
    to_child = 0;
    error_master_pkg = 0;
    error_can_not_find_child = 0;
    error_client_pkg = 0;
    warning_vanish_client = 0;
    erase_count = 0;
    retry_count = 0;
    last_print = t;
}
