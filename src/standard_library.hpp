#ifndef STANDARD_LIBRARY_HPP
#define STANDARD_LIBRARY_HPP

#include "vm.hpp"
#include "function.hpp"
#include "memory.hpp"

void Machine::initialize_standard_library() {

    #define ADD_FUNCTION(body, fname, arg_list, opcode) \
do { \
    Function function; \
    function.args = arg_list; \
    function.vm.init_og( opcode ); \
    body.members[fname] = heap.add(funValue(heap.fn_add(function))); \
} while (0)
    typedef std::vector<OpcodeObject> OPS;

    // standard library
    scopes.push_back(std::map<std::string, size_t>());
    templates.push_back(std::map<std::string, ClassTemplate>());

    // USER.OUT
    {
        Value user_out;
        user_out.type = INSTANCE;
        
        // USER.OUT.PRINT
        ADD_FUNCTION(user_out, "print", {"message"}, (
            OPS {
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_PRINT_POP)
            }
        ));
        // USER.OUT.PRINTLN
        ADD_FUNCTION(user_out, "println", {"message"}, (
            OPS {
                newOpcode(OP_BEGIN_SCOPE),
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(strValue("\n")),
                newOpcode(OP_MODULO),
                newOpcode(OP_PRINT_POP),
                newOpcode(OP_END_SCOPE),
            }
        ));


        // USER.IN
        Value user_in;
        user_in.type = INSTANCE;
        // USER.IN.READ
        ADD_FUNCTION(user_in, "read", {"buffer"}, (
            OPS {
                newOpcode(OP_BEGIN_SCOPE),
                OpConstant(idenValue("buffer")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_DEREFERENCE),
                newOpcode(OP_INPUT),
                newOpcode(OP_EDIT_VARIABLE),
                newOpcode(OP_END_SCOPE),
            }
        ));
        // USER.IN.INPUT
        ADD_FUNCTION(user_in, "input", {"message"}, (
            OPS {
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_PRINT_POP),
                newOpcode(OP_INPUT),
                newOpcode(OP_RETURN_POP),
            }
        ));

        // USER
        Value user;
        user.type = INSTANCE;
        user.members["out"] = heap.add(user_out);
        user.members["in"] = heap.add(user_in);
        scopes[0]["User"] = heap.add(user);
    }

    // DEBUG
    {
        Value debug;
        debug.type = INSTANCE;

        // DEBUG.PRINT_SCOPES
        ADD_FUNCTION(debug, "print_scopes", {}, (
            OPS {
                newOpcode(OP_DEBUG_SCOPES)
            }
        ));

        // DEBUG.PRINT_HEAP
        // not implemented

        scopes[0]["Debug"] = heap.add(debug);
    }
}

#endif