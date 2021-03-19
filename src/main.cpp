#include "vm.hpp"
#include "parserv2.hpp"
#include "lexertk.hpp"
#include "runtime.hpp"
#include "flags.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

int main(int argc, char ** argv) {
    if (argc >= 3) {
        if (std::string(argv[2]) == "t" || std::string(argv[2]) == "true") {
            flags::warnings = true;
        }
    }

    std::ifstream file(argv[1]);
    std::stringstream buf;
    buf << file.rdbuf();


    lexertk::generator generator;
    generator.next_token().type;

    if (!generator.process(buf.str())) error(std::string("Failed to lex: ") + argv[1]);

    lexertk::helper::dump(generator);

    Machine vm;
    vm.init(generator);
    vm.disassemble();
    /*vm.init_og(
        { 
            /*newOpcode(OP_BEGIN_SCOPE),
            OpConstant(strValue("hello, world! (but in a variable this time!)\n")),
            setOpcode("var"),
            OpConstant(idenValue("var")),
            newOpcode(OP_REFERENCE),
            newOpcode(OP_DEREFERENCE),
            newOpcode(OP_PRINT_POP),
            OpConstant(idenValue("var")),

            newOpcode(OP_END_SCOPE)*\/
            OpConstant(intValue(5)),
            OpConstant(floatValue(2.34)),
            newOpcode(OP_SUBTRACT),
            newOpcode(OP_PRINT_POP)
        }
    );*/
    int ret = vm.run().getInt();
    return ret;
}