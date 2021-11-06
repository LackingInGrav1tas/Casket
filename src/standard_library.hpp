#ifndef STANDARD_LIBRARY_HPP
#define STANDARD_LIBRARY_HPP

#include "vm.hpp"
#include "function.hpp"
#include "memory.hpp"

void Machine::initialize_standard_library() {
    // standard library
    scopes.push_back(std::map<std::string, size_t>());
    templates.push_back(std::map<std::string, ClassTemplate>());

    // CONSOLE.OUT
    {
        Value console_out;
        console_out.type = INSTANCE;
        
        // CONSOLE.OUT.PRINT
        Function console_out_print;
        console_out_print.args = { "message" };
        std::vector<OpcodeObject> cop_opcode = {
            OpConstant(idenValue("message")),
            newOpcode(OP_GET_VARIABLE),
            newOpcode(OP_PRINT_POP)
        };
        console_out_print.vm.init_og(cop_opcode);
        console_out.members["print"] = heap.add(funValue(heap.fn_add(console_out_print)));
        // CONSOLE.OUT.PRINTLN
        Function console_out_println;
        console_out_println.args = { "message" };
        std::vector<OpcodeObject> copln_opcode = {
            newOpcode(OP_BEGIN_SCOPE),
            OpConstant(idenValue("message")),
            newOpcode(OP_GET_VARIABLE),
            OpConstant(strValue("\n")),
            newOpcode(OP_MODULO),
            newOpcode(OP_PRINT_POP),
            newOpcode(OP_END_SCOPE),
        };
        console_out_println.vm.init_og(copln_opcode);
        console_out.members["println"] = heap.add(funValue(heap.fn_add(console_out_println)));
        // adding console.out


        // CONSOLE.IN
        Value console_in;
        console_in.type = INSTANCE;
        // CONSOLE.IN.READ
        Function console_in_read;
        console_in_read.args = { "buffer" };
        std::vector<OpcodeObject> cirl_opcode = {
            newOpcode(OP_BEGIN_SCOPE),
            OpConstant(idenValue("buffer")),
            newOpcode(OP_GET_VARIABLE),
            newOpcode(OP_DEREFERENCE),
            newOpcode(OP_INPUT),
            newOpcode(OP_EDIT_VARIABLE),
            newOpcode(OP_END_SCOPE),
        };
        console_in_read.vm.init_og(cirl_opcode);
        console_in.members["read"] = heap.add(funValue(heap.fn_add(console_in_read)));
        // CONSOLE.IN.INPUT
        Function console_in_input;
        console_in_input.args = { "message" };
        std::vector<OpcodeObject> cii_opcode = {
            OpConstant(idenValue("message")),
            newOpcode(OP_GET_VARIABLE),
            newOpcode(OP_PRINT_POP),
            newOpcode(OP_INPUT),
            newOpcode(OP_RETURN_POP),
        };
        console_in_input.vm.init_og(cii_opcode);
        console_in.members["input"] = heap.add(funValue(heap.fn_add(console_in_input)));

        // CONSOLE
        Value console;
        console.type = INSTANCE;
        console.members["out"] = heap.add(console_out);
        console.members["in"] = heap.add(console_in);
        scopes[0]["Console"] = heap.add(console);
    }

    // PYTHON
    {
        Value python;
        python.type = INSTANCE;

        /* // PYTHON.RUN
        Function python_run;
        python_run.args = { "code" };
        std::vector<OpcodeObject> pyrun_opcode = {
            OpConstant(idenValue("code")),
            newOpcode(OP_GET_VARIABLE),
            // OP_RUN_PYTHON
        };
        python_run.vm.init_og(pyrun_opcode);
        int pyrun = heap.fn_add(python_run);
        python.members["run"] = pyrun;

        scopes[0]["Python"] = heap.add(python); */
    }

    // DEBUG
    {
        Value debug;
        debug.type = INSTANCE;

        // DEBUG.PRINT_SCOPES
        Function debug_stack;
        std::vector<OpcodeObject> debug_stack_opcodes = {
            newOpcode(OP_DEBUG_SCOPES)
        };
        debug_stack.vm.init_og(debug_stack_opcodes);
        debug.members["print_scopes"] = heap.add(funValue(heap.fn_add(debug_stack)));

        // DEBUG.PRINT_HEAP
        Function print_heap;
        std::vector<OpcodeObject> ps = {
            newOpcode(OP_DEBUG_SCOPES)
        };
        print_heap.vm.init_og(ps);
        debug.members["print_scopes"] = heap.add(funValue(heap.fn_add(print_heap)));

        scopes[0]["Debug"] = heap.add(debug);
    }
}

#endif