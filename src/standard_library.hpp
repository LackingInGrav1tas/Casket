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
        ADD_FUNCTION(stream_out, "print", {"$message"}, (
            OPS {
                OpConstant(idenValue("$message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_PRINT_POP)
            }
        ));
        // STREAM.OUT.PRINTLN
        ADD_FUNCTION(stream_out, "println", {"$message"}, (
            OPS {
                OpConstant(idenValue("$message")),
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
        ADD_FUNCTION(stream_in, "read", {"$buffer"}, (
            OPS {
                OpConstant(idenValue("$buffer")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_DEREFERENCE),
                newOpcode(OP_INPUT),
                newOpcode(OP_EDIT_VARIABLE),
            }
        ));
        // STREAM.IN.INPUT
        ADD_FUNCTION(stream_in, "input", {"$message"}, (
            OPS {
                OpConstant(idenValue("$message")),
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
        ADD_FUNCTION(stream_file, "read", {"$file"}, (
            OPS {
                OpConstant(idenValue("$file")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_READ),
                newOpcode(OP_RETURN_POP),
            }
        ));
        // STREAM.FILE.READ_BYTES
        ADD_FUNCTION(stream_file, "read_bytes", {"$file"}, (
            OPS {
                OpConstant(idenValue("$file")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_READ_BYTE),
                newOpcode(OP_RETURN_POP),
            }
        ));
        // STREAM.FILE.WRITE
        ADD_FUNCTION(stream_file, "write", (std::vector<std::string> {"$file", "$message"}), (
            OPS {
                OpConstant(idenValue("$file")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("$message")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_STREAM_FILE_WRITE),
            }
        ));
        // STREAM.FILE.WRITE
        ADD_FUNCTION(stream_file, "write_bytes", (std::vector<std::string> {"$file", "$message"}), (
            OPS {
                OpConstant(idenValue("$file")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("$message")),
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

        // ENVIRONMENT.ARGS
        ADD_FUNCTION(environment, "exit", {"$val"}, (
            OPS {
                OpConstant(idenValue("$val")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_ENVIRON_EXIT),
            }
        ));

        // ENVIRONMENT.CWD
        ADD_FUNCTION(environment, "cwd", (std::vector<std::string>{}), (
            OPS {
#if defined(_WIN32) || defined(_WIN64)
                OpConstant(strValue("cd")),
                newOpcode(OP_ENVIRON_COMMAND),
                newOpcode(OP_RETURN_POP),
#else
                OpConstant(strValue("Environment.cwd() is only supported with Windows")),
                newOpcode(OP_PRINT_POP),
                OpConstant(intValue(1)),
                newOpcode(OP_ENVIRON_EXIT),
#endif
            }
        ));

        // ENVIRONMENT.CASKET
        ADD_FUNCTION(environment, "casket", {"$code"}, (
            OPS {
                OpConstant(idenValue("$code")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_ENVIRON_CASKET),
                newOpcode(OP_RETURN_POP)
            }
        ));

        // ENVIRONMENT.COMMAND
        ADD_FUNCTION(environment, "command", {"$cmd"}, (
            OPS {
                OpConstant(idenValue("$cmd")),
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

    // TYPES - enum
    {
        Value types;
        types.type = INSTANCE;

        Value prims;
        prims.type = INSTANCE;

        // setting prim types
        prims.members["INT"] = heap.add(intValue(INTIGER));
        prims.members["DOUBLE"] = heap.add(intValue(DOUBLE));
        prims.members["STRING"] = heap.add(intValue(STRING));
        prims.members["BOOLEAN"] = heap.add(intValue(BOOLEAN));
        prims.members["NULL"] = heap.add(intValue(NIL));
        prims.members["BYTE"] = heap.add(intValue(BYTE));
        prims.members["POINTER"] = heap.add(intValue(POINTER));
        prims.members["LIST"] = heap.add(intValue(LIST));
        prims.members["FUNCTION"] = heap.add(intValue(FUNCTION));
        prims.members["INSTANCE"] = heap.add(intValue(INSTANCE));

        // TYPES.IS
        ADD_FUNCTION(types, "is", (std::vector<std::string>{"$val", "$prim"}), (
            OPS {
                OpConstant(idenValue("$val")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("$prim")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_TYPES_IS),
                newOpcode(OP_RETURN_POP),
            }
        ));

        // TYPES.GET_TYPE
        ADD_FUNCTION(types, "get_type", {"$val"}, (
            OPS {
                OpConstant(idenValue("$val")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_TYPES_GET_TYPE),
                newOpcode(OP_RETURN_POP),
            }
        ));

        // TYPES.TO
        ADD_FUNCTION(types, "to", (std::vector<std::string>{"$val", "$type"}), (
            OPS {
                OpConstant(idenValue("$val")),
                newOpcode(OP_GET_VARIABLE),
                OpConstant(idenValue("$type")),
                newOpcode(OP_GET_VARIABLE),
                newOpcode(OP_TYPES_TO),
                newOpcode(OP_RETURN_POP),
            }
        ));

        types.members["prims"] = heap.add(prims);
        scopes[0]["Types"] = heap.add(types);
    }
}

#endif