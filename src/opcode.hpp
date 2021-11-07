#ifndef OPCODE_HPP
#define OPCODE_HPP

#include "value.hpp"

#include <string>

enum Opcode {
    OP_BEGIN_SCOPE=100,
    OP_END_SCOPE=1,
    
    OP_CONSTANT=2,

    OP_ADD=3,
    OP_SUBTRACT=4,
    OP_MULTIPLY=5,
    OP_DIVIDE=6,
    OP_MODULO=7,

    OP_NEGATE=8,
    OP_NOT=9,
    
    OP_AND=10,
    OP_OR=11,

    OP_LESS=12,
    OP_MORE=13,
    OP_LESS_EQ=14,
    OP_MORE_EQ=15,
    OP_EQUALITY=16,
    OP_NOT_EQUAL=17,

    OP_SET_VARIABLE=18,
    OP_EDIT_VARIABLE=19,
    OP_GET_VARIABLE=20,

    OP_REFERENCE=21,
    OP_DEREFERENCE=22,

    OP_CALL_FN=23,

    OP_COPY=24,

    OP_JUMP_FALSE=25,
    OP_JUMP=26,

    OP_RETURN_POP=27,
    OP_PRINT_POP=28,
    OP_INPUT=39,

    OP_LABEL=29,
    OP_GOTO_LABEL=30,
    OP_ERROR=31,

    OP_DECL_CLASS=32,
    OP_CREATE_INST=33,
    OP_GET_MEMBER=34,

    OP_INDEX=35,
    OP_CREATE_LIST=36,

    OP_INCREMENT=37,
    OP_DECREMENT=38,

    OP_PYTHON_RUN=40,
    OP_DEBUG_SCOPES=41,
    OP_ENVIRON_ARGS=42,
    OP_ENVIRON_COMMAND=43,
    OP_STREAM_FILE_READ=44,
    OP_STREAM_FILE_WRITE=45,
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