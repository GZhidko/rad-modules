#include <iostream>
#include "logger.h"
#include "dispatcher.h"
#include "configuration.h"
#include "balancer.h"
#include "input_buffer.h"

int main(int argc, char * const argv[]) {
    //
    // конфигурируемся
    //
    Configuration conf;
    conf.load_opts(argc, argv);
//    std::cout << conf << std::endl;
    if (conf.print_version()) {
        std::cerr << conf.version_message;
        return 0;
    }
    if (conf.print_help()) {
        std::cerr << conf.help_message;
        return 0;
    }
    int v(conf.validate());
    if (v != 0) {
        std::cerr << "Invalid command line. (reason=" << v << ") (-h)\n" << std::endl;
        return 1;
    }
    //
    // Создаём буфер для вывода ошибок и настраиваем логер
    //
    OutputBuffer error_out;
    Logger logger(&error_out, conf.get_loglevel());
    //
    // Создаём приёмник ввода (балансировщик)
    //
    OutputBuffer output;
    Balancer balancer(&conf, &logger, &output);
    //
    // Создаём диспетчер
    //
    Dispatcher dispatcher(&logger, &balancer);
    //
    // Потомки
    //
    balancer.create_and_bind_childs(&dispatcher);
    InputBuffer input(&balancer);
    //
    // и регистрируем в нём потоки
    //
    dispatcher.registre(2, &error_out);
    dispatcher.registre(1, &output);
    dispatcher.registre(0, &input);
    //
    // Полетели
    //
//    std::cerr << dispatcher << std::endl;
    logger << loglevel<debug> << "Enter to dispatcher." << endl;
    int r(dispatcher.dispatch());
    logger << loglevel<error> << "Leave dispatcher. Reason = " << r << endl;
    //
    // Если диспетчер завершился, значит что-то произошло;
    // надо до-послать stdout и stderr
    //
    dispatcher.vanish();
    dispatcher.registre(2, &error_out);
    dispatcher.registre(1, &output);
    return
    dispatcher.dispatch();
}
