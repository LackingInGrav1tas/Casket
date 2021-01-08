#ifndef VM_HPP
#define VM_HPP

#include "opcode.hpp"
#include "error.hpp"
#include "lexertk.hpp"

#include <stack>
#include <iostream>
#include <functional>

typedef std::map<std::string, size_t> Scope;

struct Machine {
    std::vector<OpcodeObject> opcode;
    std::vector<Scope> scopes;
    std::stack<Value> stack;
    size_t ip;

    void init_og(std::vector<OpcodeObject> op) {
        opcode = op;
        ip = 0;
    }
    
    void init(lexertk::generator &gen, bool fn_parsing = false);

    void disassemble() {
        #define OP opcode[i]
        for (int i = 0; i < opcode.size(); i++) {
            std::cout << i << ": ";
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
            }
            std::cout << std::endl;
        }
    }

    Value run();

};

#endif