#include "vm.hpp"
#include "parserv2.hpp"
#include "lexertk.hpp"
#include "runtime.hpp"
#include "flags.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char ** argv) {

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "warn" || arg == "warnings" || arg == "warning" || arg == "w") {
            flags::warnings = true;
        } else if (arg == "d" || arg == "debug") {
            flags::debug = true;
        }
    }

    std::ifstream file(argv[1]);
    std::stringstream buf;
    buf << file.rdbuf();


    lexertk::generator generator;

    if (!generator.process(buf.str())) error(std::string("Failed to lex: ") + argv[1]);

    Machine vm;
    vm.init(generator);
    if (flags::debug) {
        vm.disassemble();
        lexertk::helper::dump(generator);
    }
    return vm.run().getInt();
}