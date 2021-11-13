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

    // STREAM
    {
        Value stream_out;
        stream_out.type = INSTANCE;
        
        // STREAM.OUT.PRINT
        ADD_FUNCTION(stream_out, "print", {"message"}, (
            OPS {
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_PRINT_POP)
            }
        ));
        // STREAM.OUT.PRINTLN
        ADD_FUNCTION(stream_out, "println", {"message"}, (
            OPS {
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(strValue("\n")),
                newOpcode(OP_MODULO),
                newOpcode(OP_PRINT_POP),
            }
        ));


        // STREAM.IN
        Value stream_in;
        stream_in.type = INSTANCE;
        // STREAM.IN.READ
        ADD_FUNCTION(stream_in, "read", {"buffer"}, (
            OPS {
                OpConstant(idenValue("buffer")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_DEREFERENCE),
                newOpcode(OP_INPUT),
                newOpcode(OP_EDIT_VARIABLE),
            }
        ));
        // STREAM.IN.INPUT
        ADD_FUNCTION(stream_in, "input", {"message"}, (
            OPS {
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_PRINT_POP),
                newOpcode(OP_INPUT),
                newOpcode(OP_RETURN_POP),
            }
        ));

        // STREAM.FILE
        Value stream_file;
        stream_file.type = INSTANCE;        
        // STREAM.FILE.READ
        ADD_FUNCTION(stream_file, "read", {"file"}, (
            OPS {
                OpConstant(idenValue("file")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_READ),
                newOpcode(OP_RETURN_POP),
            }
        ));
        // STREAM.FILE.READ_BYTES
        ADD_FUNCTION(stream_file, "read_bytes", {"file"}, (
            OPS {
                OpConstant(idenValue("file")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_READ_BYTE),
                newOpcode(OP_RETURN_POP),
            }
        ));
        // STREAM.FILE.WRITE
        ADD_FUNCTION(stream_file, "write", (std::vector<std::string> {"file", "message"}), (
            OPS {
                OpConstant(idenValue("file")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_WRITE),
            }
        ));
        // STREAM.FILE.WRITE
        ADD_FUNCTION(stream_file, "write_bytes", (std::vector<std::string> {"file", "message"}), (
            OPS {
                OpConstant(idenValue("file")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_WRITE_BYTE),
            }
        ));

        // STREAM
        Value stream;
        stream.type = INSTANCE;
        stream.members["in"]   = heap.add(stream_in);
        stream.members["out"]  = heap.add(stream_out);
        stream.members["file"] = heap.add(stream_file);
        scopes[0]["Stream"] = heap.add(stream);
    }

    // ENVIRONMENT
    {
        Value environment;
        environment.type = INSTANCE;

        // ENVIRONMENT.ARGS
        ADD_FUNCTION(environment, "args", {}, (
            OPS {
                newOpcode(OP_ENVIRON_ARGS),
                newOpcode(OP_RETURN_POP)
            }
        ));

        // ENVIRONMENT.CWD
        ADD_FUNCTION(environment, "cwd", {}, (
            OPS {
                newOpcode(OP_ENVIRON_CWD),
                newOpcode(OP_RETURN_POP)
            }
        ));

        // ENVIRONMENT.CASKET
        ADD_FUNCTION(environment, "casket", {"code"}, (
            OPS {
                OpConstant(idenValue("code")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_ENVIRON_CASKET),
                newOpcode(OP_RETURN_POP)
            }
        ));

        // ENVIRONMENT.COMMAND
        ADD_FUNCTION(environment, "command", {"cmd"}, (
            OPS {
                OpConstant(idenValue("cmd")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_ENVIRON_COMMAND),
                newOpcode(OP_RETURN_POP)
            }
        ));

        scopes[0]["Environment"] = heap.add(environment);
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