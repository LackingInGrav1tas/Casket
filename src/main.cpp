#include "lexer.hpp"
#include "vm.hpp"
#include "parserv2.hpp"
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

    
    if (flags::load) {
        if (flags::debug) std::cout << "LOADING" << std::endl;

        char *serialized;
        // std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
        // std::cout << "tellg" << file.tellg() << std::endl;
        // file.read(serialized, file.tellg());
        std::cout << "reinterpreting" << std::endl;
        std::ifstream(argv[1]) >> serialized;
        Machine vm = *reinterpret_cast<Machine*>(serialized);
        std::cout << "running\n" << vm.opcode.size() << "\n" << vm.ip << std::endl;
        return vm.run().getInt();
    } else if (flags::compile) {
        if (flags::debug) std::cout << "COMPILING" << std::endl;
        std::ifstream file(argv[1]);
        std::stringstream buf;
        buf << file.rdbuf();

        Lexer lexer(buf.str());

        Machine vm;
        // std::cout << sizeof(vm) << "\n" << sizeof(vm.opcode) << std::endl;
        vm.init(lexer);
        vm.serialize(std::ifstream(argv[2]));
        // std::ofstream(argv[2]) << reinterpret_cast<const char*>(&vm);
        /*const char * serialized = reinterpret_cast<const char*>(&vm);
        std::ofstream(argv[2]).write(serialized, strlen(serialized));*/
    } else {
        std::ifstream file(argv[1]);
        std::stringstream buf;
        buf << file.rdbuf();

        Lexer lexer(buf.str());

        std::cout << "\nLEXING DONE" << std::endl;
        Machine vm;
        vm.init(lexer);
        std::cout << "\nPARSING DONE\n" << std::endl;
        int v = vm.run().getInt();
        std::cout << "\n\nSUCCESS" << std::endl;
        return v;
    }
}