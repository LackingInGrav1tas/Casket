#include "vm.hpp"

#include <fstream>
#include <iostream>

int main(int argc, char ** argv) {
    Machine vm;
    vm.init(
        { 
            newOpcode(OP_BEGIN_SCOPE),
            OpConstant(strValue("hello, world! (but in a variable this time!)\n")),
            setOpcode("var"),
            OpConstant(idenValue("var")),
            newOpcode(OP_REFERENCE),
            newOpcode(OP_DEREFERENCE),
            newOpcode(OP_PRINT_POP),
            OpConstant(idenValue("var")),
            newOpcode(OP_END_SCOPE)
        }
    );
    return vm.run().getInt();
}