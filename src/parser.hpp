#ifndef PARSER_HPP
#define PARSER_HPP

#include <functional>

#include "vm.hpp"
#include "lexertk.hpp"
#include "error.hpp"

typedef lexertk::generator Generator;
typedef lexertk::token Token;
typedef lexertk::token::token_type Type;

static int getPrecedence(Type t) {
    switch (t) {
        case Type::e_ne          : return 4;
        case Type::e_eq          : return 4;
        case Type::e_lte         : return 5;
        case Type::e_gte         : return 5;
        case Type::e_lt          : return 5;
        case Type::e_gt          : return 5;
        case Type::e_add         : return 6;
        case Type::e_sub         : return 6;
        case Type::e_div         : return 7;
        case Type::e_mul         : return 7;
        case Type::e_mod         : return 7;
        case Type::e_pow         : return 8;
        case Type::e_rbracket    : return 9;
        case Type::e_lbracket    : return 9;
        default            : return 0;
    }
}


void Machine::init(Generator gen) {
    ip = 0;
    int loc = -1;

    #define ADV() gen.next_token()
    #define NEXT() \
        loc++; \
        if (gen.peek_next_token().is_error()) \
            error("parsing error: " + Token::to_str(gen.peek_next_token().type)); \
        Token current = ADV()
    #define CASE(t) (current.type == t)
    #define PEEK() (gen.peek_next_token())
    #define PUSH(arg) opcode.push_back(newOpcode(arg))
    #define PUSHC(arg) opcode.push_back(OpConstant(arg))
    #define EXP(n) if (!gen.empty()) expression(n); else error("run-time error: expected an expression");

    std::function<void(int)> expression = [&](int p)->void {
        NEXT();
        if CASE(Type::e_lbracket) { // group
            gen.next_token();
            expression(1);
            if (PEEK().type != Type::e_rbracket) {
                error("parsing error: expected a ')'  token: " + PEEK().value);
            }
            loc++;
            ADV();
        } else if CASE(Type::e_sub) {
            expression(7);
            PUSH(OP_NEGATE);
        } else if CASE(Type::e_exclamation) {
            expression(7);
            PUSH(OP_NOT);
        } else if (current.type == Type::e_symbol && current.value == "set") {
            return;
        } else if (current.type == Type::e_symbol && current.value == "true") {
            PUSHC(boolValue(true));
        } else if (current.type == Type::e_symbol && current.value == "false") {
            PUSHC(boolValue(false));
        } else if (current.type == Type::e_symbol && current.value == "null") {
            PUSHC(nullValue());
        } else if (current.type == Type::e_symbol && current.value == "print") {
            PUSH(OP_PRINT_POP);
        } else if CASE(Type::e_symbol) {
            PUSHC(idenValue(current.value));
        } else if CASE(Type::e_number) {
            int is_float = false;
            for (int i = 0; i < current.value.length(); i++) {
                if (current.value[i] == '.') {
                    is_float = true;
                    break;
                }
            }
            if (is_float) PUSHC(floatValue(std::stof(current.value)));
            else PUSHC(intValue(std::stoi(current.value)));
        } else if CASE(Type::e_string) {
            PUSHC(strValue(current.value));
        } else std::cout << "else" << std::endl;
    };
    while (loc < (int)gen.size()-1) {
        expression(1);
    }
}

#endif