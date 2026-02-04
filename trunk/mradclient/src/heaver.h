/*

МЫСЛЬ

   Логика работы (минимальная, текущая функциональность)
   *****************************************************

== Замечания:
 - пока map -- это не понятно, что. некое хранилище.

== Когда получаем пакет от радиуса, мы должны выбрать
   потомка, которому отдадим запрос.

 - выбираем слот по ID
 - смотрим, есть ли в map Authenticator (* доступ к map по Authenticator *)
   - если есть, то мы имеем дело с retry:
     - обновляем дату отправки
     - отправляем тому потомку, который записан в map
   - если нет, то мы имеем дело с новым пактом:
     - начинаем перебирать возможных потомков начиная
       с текущего (finger)
     - когда находим потомка, которого нет в map: (* доступ к map по child_id *)
       - заводим для этого потомка запись в map
       - отправляем пакет этому потомку
     - если в map всё занято, то проверяем time-out-ы,
       если есть, что почистить, то:
       - вычищаем устаревшую запись и создаём на её месте
         новую
     - если ничего не устарело и всё переполнено, то отдаём
       радиусу пакет с ошибкой и пишем лог

== Когда получаем пакет от клиента.
 - выбираем слот по ID
 - выбираем запись по Authenticator или child_id (* какой доступ к map лучше? *)
 - удаляем эту запись
 - отправляем пакет радиусу

== итого. как устроен map:
 - в ноде хранится:
   - child_id
   - time_stamp
 - доступ к ноде по
   - Authenticator: получаем child_id и обновляем time_stamp
   - по child_id: только определить есть или нет
 - в map всегда уникальны атрибуты, по которым осуществляется доступ
   - Authenticator
   - по child_id

== Такое чувство, [UPD: это чувство было ложным :-) кластеры как раз выделен]
   что, возможно, не надо выделять кластеры в отдельный объект;
   может быть всё свалить немного в кучу, но от этого упростится доступ
   к, например, тайм-ауту

/МЫСЛЬ

*/

#ifndef MRADCLIENT_HEAVER_H
#define MRADCLIENT_HEAVER_H

#include <map>
#include <string>
#include <time.h>
#include "heaver_cluster.h"

class Balancer;
class Logger;
class Configuration;
class RadPackage;

enum HeaverError {
    pkt_ok,
    parser_error,
    can_not_find_child,
    warning_vanish_client
};

class Heaver {
private:
    Balancer * balancer;
    Logger * logger;
    HeaverCluster clusters[256];
    int time_out;
    std::string uniq_attr_name;
    bool remove_uniq_attr;

    void send_error_to_master(time_t const now, RadPackage const & pkg, int code) const;

public:
    Heaver(Balancer *, Logger *, Configuration *);
    Heaver(const Heaver &);             // IMPLEMENT IT!!
    Heaver & operator=(const Heaver &); // IMPLEMENT IT!!
    HeaverError send_to_slave(time_t const now, char const * const data, int const len, int & count_erase, bool & is_retry);
    HeaverError send_to_master(int cid_from, time_t const now, char const * const data, int const len);
};

#endif // MRADCLIENT_HEAVER_H
