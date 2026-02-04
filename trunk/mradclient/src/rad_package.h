#ifndef MRADCLIENT_RAD_PACKAGE_H
#define MRADCLIENT_RAD_PACKAGE_H

#include <list>
#include <iostream>
#include <string>

class RadPackage {
friend std::ostream & operator<<(std::ostream & out, const RadPackage & object);
private:
    class pkt_lines {
    public:
        std::string line; // строка
        std::string key;
        std::string value;
        pkt_lines() : line(), key(), value() {};
    };
    std::list<pkt_lines> pkt;
public:
    RadPackage(char const * data, int len);
    std::string const value(std::string const & key) const;
    int id() const;
    int remove(std::string const & key);
    std::string const as_string() const;
    ~RadPackage();
};

#endif // MRADCLIENT_RAD_PACKAGE_H
