#include "balancer.h"
#include "configuration.h"
#include "logger.h"
#include "heaver.h"
#include "rad_package.h"
#include <assert.h>
#include <sstream>

Heaver::Heaver(Balancer * b, Logger * l, Configuration * c) :
    balancer(b),
    logger(l),
    time_out(c->get_timeout()),
    uniq_attr_name(c->get_uniq_attr_name()),
    remove_uniq_attr(c->get_remove_uniq_attr_flag())
{
}

HeaverError
Heaver::send_to_slave(time_t const now, char const * const data, int const len, int & count_erase, bool & is_retry) {
    RadPackage p(data, len);
    // здесь принимаем решение, какому потомку передаём
    int identifier(p.id());
    if (identifier < 0) {
        *logger << loglevel<error> << "PARSER ERROR: Can not find RAD-Identifier" << endl;
        send_error_to_master(now, p, 99);
        return parser_error;
    }
    std::string s_uniq(p.value(uniq_attr_name /* XXX "RAD-Authenticator" */));
    if (s_uniq.empty()) {
        *logger << loglevel<error> << "PARSER ERROR: Can not find \"" << uniq_attr_name << "\"" << endl;
        send_error_to_master(now, p, 98);
        return parser_error;
    }
    // смотрим, есть ли уже такое; перепосыл ли это

//    clusters[identifier].dump(logger, identifier);
    int cid = clusters[identifier].get_child_id_and_update_record(
              s_uniq, now, balancer->pool_size(), time_out,
              count_erase, is_retry);
    *logger << loglevel<rinfo>
            << "id/a/cid = "
            << identifier << "/"
            << s_uniq << "/"
            << cid << endl;
//    clusters[identifier].dump(logger, identifier);

    if (cid < 0) {
        *logger << loglevel<error> << "id/a/cid = " <<
                   identifier << "/" <<
                   s_uniq << "/" <<
                   cid << " (DROP)\n" <<
                   clusters[identifier];
        send_error_to_master(now, p, 97);
        return can_not_find_child;
    }

    *logger << loglevel<debug>;

    if (remove_uniq_attr) {
        int c(p.remove(uniq_attr_name));
        std::string s(p.as_string());
        *logger << "POST TO Child " << cid << " (remove " << c << " pairs):\n"
                << s
                << "POST TO Child " << cid << ".\n";
        balancer->send_data_to_child(cid, now, s.c_str(), s.length());
    } else {
        *logger << "POST TO Child " << cid << ":\n"
                << std::string(data, len)
                << "POST TO Child " << cid << ".\n";
        balancer->send_data_to_child(cid, now, data, len);
    }

    return pkt_ok;
}

void
Heaver::send_error_to_master(time_t const now, RadPackage const & pkg, int code) const {
    // стараемся сохранить Radclient-Query-Id
    // можно и по-прямей переписать, но ошибки-то возникают очень редко
    std::ostringstream rep;
    rep << "int = " << code << "\n";
    std::string qid(pkg.value("Radclient-Query-Id"));
    if (! qid.empty()) {
        rep << "Radclient-Query-Id = " << qid << "\n";
    }
    rep << "\n";
    std::string r(rep.str());
    *logger << loglevel<error>
            << "Packet droped; reply to core:\n"
            << r
            << ".\n";
    balancer->send_data_to_master(now, r.c_str(), r.length());
}

HeaverError
Heaver::send_to_master(int cid_from, time_t const now, char const * const data, int const len) {
    // здесь обрабатываем, от какого потомка приняли
    // и передаём мастеру

    *logger << loglevel<debug>
            << "GET FROM Child " << cid_from << ":\n"
            << std::string(data, len)
            << "GET FROM Child " << cid_from << ".\n";

    RadPackage p(data, len);
    int identifier(p.id());
    if (identifier < 0) {
        *logger << loglevel<error>
                << "Can not find RAD-Identifier:\n"
                << std::string(data, len)
                << ".\n";
        // мы ответ не поняли (битый)
        // но отправить его мастеру всё равно надо!
        balancer->send_data_to_master(now, data, len);
        return parser_error;
    }

    HeaverError result(pkt_ok);
    *logger << clusters[identifier];
    *logger << "Vanish (id=" << identifier << "; cid=" << cid_from << ")\n";
    int r = clusters[identifier].vanish(cid_from);
    *logger << "Vanish result = " << r << "\n";
    *logger << clusters[identifier];
    if (r == 0) {
        result = warning_vanish_client; // статистика результатов вытирания (пакет ниоткуда)
    }

    balancer->send_data_to_master(now, data, len);

    return result;
}
