#ifndef OPCODE_HPP
#define OPCODE_HPP

#include "value.hpp"

#include <string>

enum Opcode {
    OP_BEGIN_SCOPE,
    OP_END_SCOPE,
    
    OP_CONSTANT,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,

    OP_NEGATE,
    OP_NOT,
    
    OP_AND,
    OP_OR,

    OP_LESS,
    OP_MORE,
    OP_LESS_EQ,
    OP_MORE_EQ,
    OP_EQUALITY,
    OP_NOT_EQUAL,

    OP_SET_VARIABLE,
    OP_EDIT_VARIABLE,
    OP_GET_VARIABLE,

    OP_REFERENCE,
    OP_DEREFERENCE,

    OP_CALL_FN,

    OP_COPY,

    OP_JUMP_FALSE,
    OP_JUMP_LOOP,
    OP_JUMP,
    OP_BREAK,
    OP_CONTINUE,

    OP_RETURN_POP,
    OP_PRINT_POP,
    OP_INPUT,

    OP_LABEL,
    OP_GOTO_LABEL,
    OP_ERROR,

    OP_DECL_CLASS,
    OP_CREATE_INST,
    OP_GET_MEMBER,

    OP_INDEX,
    OP_CREATE_LIST,

    OP_INCREMENT,
    OP_DECREMENT,

    OP_DEBUG_SCOPES,
    OP_ENVIRON_ARGS,
    OP_ENVIRON_COMMAND,
    OP_STREAM_FILE_READ,
    OP_STREAM_FILE_WRITE,
    OP_ENVIRON_CWD,
    OP_STREAM_FILE_READ_BYTE,
    OP_STREAM_FILE_WRITE_BYTE,

    OP_BIT_AND,
    OP_BIT_OR,
    OP_XOR,

    OP_ENVIRON_CASKET,
    OP_ENVIRON_EXIT,

    OP_TYPES_IS,
    OP_TYPES_GET_TYPE,
    OP_TYPES_TO,
};

struct OpcodeObject {
    Opcode op;
    std::string lexeme;
    int i;
    Value value;

    int size() {
        return sizeof(Opcode) + sizeof(int);
    }
};

OpcodeObject newOpcode(Opcode op) {
    OpcodeObject o;
    o.op = op;
    return o;
}

OpcodeObject spOpcode(Opcode op, std::string identifier) {
    OpcodeObject o;
    o.op     = op;
    o.lexeme = identifier;
    return o;
}

OpcodeObject callOpcode(int i) {
    OpcodeObject o;
    o.op = OP_CALL_FN;
    o.i  = i;
    return o;
}

OpcodeObject jumpOpcode(Opcode op, int i) {
    OpcodeObject o;
    o.op = op;
    o.i  = i;
    return o;
}

OpcodeObject OpConstant(Value v) {
    OpcodeObject o;
    o.op    = OP_CONSTANT;
    o.value = v;
    return o;
}

OpcodeObject DeclClassOpcode(std::string classname, int i) {
    OpcodeObject o;
    o.op     = OP_DECL_CLASS;
    o.lexeme = classname;
    o.i      = i;
    return o;
}

OpcodeObject InstanceOpcode(std::string name, int i = 0) {
    OpcodeObject o;
    o.op = OP_CREATE_INST;
    o.lexeme = name;
    o.i = i;
    return o;
}

OpcodeObject ListOpcode(int i) {
    OpcodeObject o;
    o.op = OP_CREATE_LIST;
    o.i = i;
    return o;
}

#endif