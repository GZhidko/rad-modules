#include "rad_package.h"
#include <string.h>
#include <sstream>
#include <iostream>
#include <ctype.h>
#include <assert.h>

RadPackage::RadPackage(char const * d, int l) :
    pkt()
{
    pkt_lines p;
    int state(0);

// надо подумать, как это сделать красиво
// - можно взять исходники радиуса и использовать тамошний партсер
// - может быть вообще перейти на бинарный протокол

    for (const char *i(d), *e(d + l); i<e; ++i) {
//        std::cerr << "[" << *i << "]";
        switch(state) {
            case 0:
                p.line.clear();
                state = 1;
                // no break
            case 1:
                p.line += *i;
                if (! isspace(*i)) {
                    p.key = *i;
                    state = 2;
                }
                break;
            case 2:
                p.line += *i;
                if (isspace(*i) || *i == '=') {
                    state = 3;
                } else {
                    p.key += *i;
                }
                break;
            case 3:
                p.line += *i;
                if (!(isspace(*i) || *i == '=')) {
                    p.value = *i;
                    state = 4;
                }
                break;
            case 4:
                p.line += *i;
                if (*i == '\n') {
                    pkt.push_back(p);
//                  p.line.clear();
//                  p.key.clear();
//                  p.value.clear();
                    state = 0;
                } else {
                    p.value += *i;
                }
                break;
            default:
                assert(true);
        }
//        std::cerr << " [" << state << "]";
//      std::cerr << std::endl;
    }
}

RadPackage::~RadPackage() {
}

std::string const
RadPackage::value(std::string const & k) const {
    for(std::list<RadPackage::pkt_lines>::const_iterator i(pkt.begin()), e(pkt.end()); i != e; ++i) {
        if ((*i).key.find(k) != std::string::npos) {
            return (*i).value;
        }
    }
    return std::string();
}

int
RadPackage::id() const {
    std::istringstream i(value("RAD-Identifier"));
    int v;
    if (i >> v) {
        if (v > 255 || v < 0) {
            return -1;
        }
        return v;
    }
    return -1;
}

int
RadPackage::remove(std::string const & k) {
    int c(0);
    for(std::list<RadPackage::pkt_lines>::iterator i(pkt.begin()), e(pkt.end()); i != e;) {
        if ((*i).key.find(k) != std::string::npos) {
            i = pkt.erase(i);
            ++c;
        } else {
            ++i;
        }
    }
    return c;
}

std::string const
RadPackage::as_string() const {
    std::string s;
    for(std::list<RadPackage::pkt_lines>::const_iterator i(pkt.begin()); i != pkt.end(); ++i) {
        s.append((*i).line);
    }
    s.append("\n");
    return s;
}

// === DEBUGGING AND TESTING ONLY ===

std::ostream &
operator<<(std::ostream & out, const RadPackage & obj) {
    out << "--- DUMP ---" << std::endl;
    for(std::list<RadPackage::pkt_lines>::const_iterator i(obj.pkt.begin()); i != obj.pkt.end(); ++i) {
        out << "L=["
            << i->line
            << "]"
            << "K=["
            << i->key
            << "]"
            << "V=["
            << i->value
            << "]"
            << std::endl;
    }
    out << "--- /DUMP ---" << std::endl;
    return out;
}

#ifdef DEBUG_rad_package
// -*- TEST -*-
//  g++ -DDEBUG_rad_package rad_package.cpp && echo '--test--' && ./a.out && echo '--done--'
// -*-
int main() {
    const char *c = "  kaka = KAKA---1111\n"
                    "  kaka = KAKA---2222\n"
                    "  abc =\"ABC\"\n"
                    "  :RAD-Identifier = 209\n"
                    "\n";
    int l(strlen(c));
    RadPackage::RadPackage p(c, l);
    std::cerr << "=" << p.value("2kaka") << "=" << p.id() << "=" << std::endl;
    std::cerr << p;
    std::cerr << "=remove=" << std::endl
              << "n=" << p.remove("kaka") << std::endl
              << p;
    std::cerr << "=string=" << std::endl
              << "[[" << p.as_string() << "]]"
              << std::endl;
}
#endif
