#include "lexer.hpp"
#include "vm.hpp"
#include "parserv2.hpp"
#include "standard_library.hpp"
#include "runtime.hpp"
#include "flags.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>

int main(int argc, char ** argv) {
    if (argc <= 1) {
        std::cerr << "correct format: " << (std::string)argv[0] << " <file> [warnings] [debug] [collect]" <<std::endl;
    }

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "warn" || arg == "warnings" || arg == "warning" || arg == "w")
            flags::warnings = true;
        else if (arg == "d" || arg == "debug")
            flags::debug = true;
        else if (arg == "gc" || arg == "collect")
            flags::collect = true;
        else if (arg == "c" || arg == "compile")
            flags::compile = true;
        else if (arg == "l" || arg == "load")
            flags::load = true;
    }

    heap.init();
    std::ifstream file(argv[1]);
    std::stringstream buf;
    buf << file.rdbuf();

    Lexer lexer("" + buf.str());

    std::cout << "\nLEXING DONE" << std::endl;
    Machine vm;
    vm.init(lexer);
    vm.initialize_standard_library();
    std::cout << "\nPARSING DONE\n" << std::endl;
    int v = vm.run(argc, argv).getInt();
    std::cout << "\n\nSUCCESS" << std::endl;
    return v;
}