#ifndef VM_HPP
#define VM_HPP

#include "opcode.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "classes.hpp"

#include <stack>
#include <iostream>
#include <functional>
#include <fstream>

typedef std::map<std::string, size_t> Scope;

void printOp(OpcodeObject OP) {
    if (OP.op == OP_CONSTANT) {
        std::cout << "OP_CONSTANT(" << OP.value.toString() << ")";
    } else if (OP.op == OP_PRINT_POP) {
        std::cout << "OP_PRINT_POP";
    } else if (OP.op == OP_NEGATE) {
        std::cout << "OP_NEGATE";
    } else if (OP.op == OP_ADD) {
        std::cout << "OP_ADD";
    } else if (OP.op == OP_SUBTRACT) {
        std::cout << "OP_SUBTRACT";
    } else if (OP.op == OP_MULTIPLY) {
        std::cout << "OP_MULTIPLY";
    } else if (OP.op == OP_SET_VARIABLE) {
        std::cout << "OP_SET_VARIABLE(" << OP.lexeme << ")";
    } else if (OP.op == OP_GET_VARIABLE) {
        std::cout << "OP_GET_VARIABLE";
    } else if (OP.op == OP_BEGIN_SCOPE) {
        std::cout << "OP_BEGIN_SCOPE";
    } else if (OP.op == OP_END_SCOPE) {
        std::cout << "OP_END_SCOPE";
    } else if (OP.op == OP_NOT) {
        std::cout << "OP_NOT";
    } else if (OP.op == OP_LESS) {
        std::cout << "OP_LESS";
    } else if (OP.op == OP_LESS_EQ) {
        std::cout << "OP_LESS_EQ";
    } else if (OP.op == OP_MORE) {
        std::cout << "OP_MORE";
    } else if (OP.op == OP_MORE_EQ) {
        std::cout << "OP_MORE_EQ";
    } else if (OP.op == OP_EQUALITY) {
        std::cout << "OP_EQUALITY";
    } else if (OP.op == OP_NOT_EQUAL) {
        std::cout << "OP_NOT_EQUAL";
    } else if (OP.op == OP_REFERENCE) {
        std::cout << "OP_REFERENCE";
    } else if (OP.op == OP_DEREFERENCE) {
        std::cout << "OP_DEREFERENCE";
    } else if (OP.op == OP_EDIT_VARIABLE) {
        std::cout << "OP_EDIT_VARIABLE";
    } else if (OP.op == OP_COPY) {
        std::cout << "OP_COPY";
    } else if (OP.op == OP_CALL_FN) {
        std::cout << "OP_CALL_FN";
    } else if (OP.op == OP_JUMP_FALSE) {
        std::cout << "OP_JUMP_FALSE(" << OP.i << ")";
    } else if (OP.op == OP_JUMP) {
        std::cout << "OP_JUMP(" << OP.i << ")";
    } else if (OP.op == OP_ERROR) {
        std::cout << "OP_ERROR";
    } else if (OP.op == OP_LABEL) {
        std::cout << "OP_LABEL(" + OP.lexeme << ")";
    } else if (OP.op == OP_GOTO_LABEL) {
        std::cout << "OP_GOTO_LABEL(" << OP.lexeme << ")";
    } else if (OP.op == OP_DECL_CLASS) {
        std::cout << "OP_DECL_CLASS(" << OP.lexeme << ", " << OP.i << ")";
    } else if (OP.op == OP_GET_MEMBER) {
        std::cout << "OP_GET_MEMBER";
    } else if (OP.op == OP_CREATE_INST) {
        std::cout << "OP_CREATE_INST(" << OP.lexeme << ")";
    } else if (OP.op == OP_RETURN_POP) {
        std::cout << "OP_RETURN_POP";
    } else if (OP.op == OP_DIVIDE) {
        std::cout << "OP_DIVIDE";
    } else if (OP.op == OP_AND) {
        std::cout << "OP_AND";
    } else if (OP.op == OP_OR) {
        std::cout << "OP_OR";
    } else {
        std::cout << OP.op;
    }
}

struct Machine {
    std::vector<std::map<std::string, ClassTemplate>> templates;
    std::vector<OpcodeObject> opcode;
    std::vector<Scope> scopes;
    std::stack<Value> stack;
    size_t ip;

    void init_og(std::vector<OpcodeObject> op) {
        opcode = op;
        ip = 0;
    }
    
    void init(Lexer &lexer, bool fn_parsing = false);

    void initialize_standard_library();

    void disassemble() {
        #define OP opcode[i]
        std::cout << std::endl;
        for (int i = 0; i < opcode.size(); i++) {
            std::cout << i << ": ";
            printOp(OP);
            std::cout << "  --  " << OP.op << std::endl;
        }
    }

    Value run();

};

#endif