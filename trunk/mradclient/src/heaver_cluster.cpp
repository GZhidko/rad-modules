#include "heaver_cluster.h"
#include "logger.h"

HeaverCluster::HeaverCluster() :
    authenticators(),
    child_ids(),
    finger(0),
    last_clean(0)
{
}

HeaverCluster::~HeaverCluster() {
    for(std::map<int, pkg_record_t*>::const_iterator i(child_ids.begin()), e(child_ids.end()); i != e; ++i) {
        delete (*i).second;
    }
}


// годен и для связки с std::err может шаблон сделать?
//template<class S>
//S &
//operator<< (S & l, const HeaverCluster & o) {
Logger &
operator<<(Logger & l, const HeaverCluster & o) {
    l << "  child_ids:\n";
    for(std::map<int, HeaverCluster::pkg_record_t*>::const_iterator i(o.child_ids.begin());
        i != o.child_ids.end();
        ++i) {
        l << "    " << (*i).first << ": " <<
                       (*i).second->child_id << " " <<
                       (*i).second->stamp << " " <<
                       (*i).second->authenticator << "\n";
    }
    l << "  authenticators:\n";
    for(std::map<std::string, HeaverCluster::pkg_record_t*>::const_iterator i(o.authenticators.begin());
        i != o.authenticators.end();
        ++i) {
        l << "    " << (*i).first.c_str() << ": " <<
                       (*i).second->child_id << " " <<
                       (*i).second->stamp << " " <<
                       (*i).second->authenticator << "\n";
    }
    l << "  finger = " << o.finger << "\n"
         "  .\n";
    return l;
}


//Logger & operator<<(Logger & l, const HeaverCluster & o);

int
HeaverCluster::vanish(int cid) {
    std::map<int, pkg_record_t*>::iterator i(child_ids.find(cid));
    if (i == child_ids.end()) {
        return 0;
    }
    // необходимость этой проверки весьма сомнительна.
    // данные не должны правильно создаваться.
    // не должна заводиться запись поверх записи.
    // это может возникнуть только при редактировании кода.
    std::map<std::string, pkg_record_t*>::iterator j(authenticators.find((*i).second->authenticator));
    if (j != authenticators.end()) {
        authenticators.erase(j);
    }
    child_ids.erase(i);
    delete (*i).second;
    return 1;
}

int
HeaverCluster::get_child_id_and_update_record(std::string const & a, time_t now, int pool_size, int time_out,
                                              int & count_erase, bool & is_retry) {
    // ***
    // *** чистка устаревших записей
    // ***
    count_erase = 0;
    if (now - last_clean > 10) { // чистим каждые 10 секунд
        for(std::map<int, pkg_record_t*>::iterator i(child_ids.begin()), e(child_ids.end()); i != e;) {
            if (now - (*i).second->stamp > time_out) {
                authenticators.erase(authenticators.find((*i).second->authenticator));
                delete (*i).second;
                child_ids.erase(i++); // именно пост- :-)
                ++count_erase; // ++ СТАТИСТИКА УДАЛИЛИ
            } else {
                ++i;
            }
        }
        last_clean = now;
    }
    // ***
    // *** поиск
    // ***
    std::map<std::string, pkg_record_t*>::iterator ait(authenticators.find(a));
    if (ait == authenticators.end()) {
        // * такой записи нет
        // * пытаемся создать новую
        // *** это место для существенной оптимизации
        // ищем не занятого потомка
        int end(finger % pool_size); // а вдруг pool_size уменьшился?
        for (;;) {
            finger = (finger + 1)%pool_size;
            std::map<int, pkg_record_t*>::iterator cit(child_ids.find(finger));
            if (cit == child_ids.end()) {
                pkg_record_t *f(new pkg_record_t());
                f->authenticator = a;
                f->child_id = finger;
                f->stamp = now;
                child_ids[finger] = f;
                authenticators[a] = f;
                is_retry = false; // ++ СТАТИСТИКА НОВЫЙ
                return finger;
            }
            if (finger == end) {
                return -102; // ошибка -- нет свободного потомка (круг замкнулся)
            }
        }
        return -101;
    }
    // обновляем время доступа
    // и возвращаем номер потомка
    (*ait).second->stamp = now;
    is_retry = true; // ++ СТАТИСТИКА ПЕРЕПОСЛАЛИ
    return (*ait).second->child_id;
}

#ifdef DEBUG_heaver_cluster
// -*- TEST -*-
//  g++ -DDEBUG_heaver_cluster heaver_cluster.cpp && echo '--test--' && ./a.out && echo '--done--'
// -*-

int main() {
}

#endif
