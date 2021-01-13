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
    OP_JUMP,

    OP_RETURN_POP,
    OP_PRINT_POP,

    OP_LABEL,
    OP_GOTO_LABEL,
    OP_ERROR,
};

struct OpcodeObject {
    Opcode op;
    std::string lexeme;
    int i;
    Value value;
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

#endif