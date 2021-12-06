#include <fstream>
#include <sstream>
#include <iostream>
#include <string.h>

#include "lexer.hpp"
#include "vm.hpp"
#include "parserv2.hpp"
#include "standard_library.hpp"
#include "runtime.hpp"
#include "flags.hpp"

#define DEBUG(stmt) if (flags::debug) stmt

int main(int argc, char ** argv) {
    // parsing args
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
            flags::collect = false;
    }

    // getting source code
    heap.init();
    std::ifstream file(argv[1]);
    std::stringstream buf;
    buf << file.rdbuf();

    Lexer lexer(buf.str() + "\n;\"\";");

    DEBUG(PRINT("\nLEXING DONE"));
    Machine vm;
    vm.init(lexer);
    vm.initialize_standard_library();
    DEBUG(PRINT("\nPARSING DONE\n"));
    int v = vm.run(argc, argv).getInt();
    DEBUG(PRINT("\n\nSUCCESS"));
    return v;
}