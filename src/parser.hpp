#ifndef PARSER_HPP
#define PARSER_HPP

#include <functional>

#include "vm.hpp"
#include "lexertk.hpp"
#include "error.hpp"
#include "function.hpp"
#include "memory.hpp"

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
        case Type::e_lbracket    : return 9;
        default                  : return 0;
    }
}

static bool invalidIdentifier(std::string id) {
    return id == "set" || id == "fn" || id == "if" || id == "for" || id == "while";
}


void Machine::init(Generator &gen, bool fn_parsing) {
    ip = 0;
    static int loc = -1;
    heap.init();

    auto ADV = [&]()->Token {
        loc++;
        return gen.next_token();
    };

    #define NEXT() \
        if (gen.peek_next_token().is_error()) \
            error("parsing error: " + gen.peek_next_token().toStr()); \
        Token current = ADV()
    #define CASE(t) (current.type == t)
    #define PEEK() (gen.peek_next_token())
    #define PUSH(arg) opcode.push_back(newOpcode(arg))
    #define PUSHC(arg) opcode.push_back(OpConstant(arg))

    std::function<void(int)> expression = [&](int p)->void {
        NEXT();
        if CASE(Type::e_lbracket) { // group
            expression(1);
            if (gen.token_itr_->type != Type::e_rbracket) {
                error("parsing error: expected a ')'  token: " + PEEK().toStr());
            }
            ADV();
        } else if CASE(Type::e_lcrlbracket) {
            while (1) {
                expression(1);
                auto v = ADV();
                if (v.value != ";") error("parsing error: expected a semicolon  token: " + v.toStr());
                if (gen.peek_next_token().type == Type::e_rcrlbracket) break;
            }
            ADV();
            return;
        } else if CASE(Type::e_sub) {
            expression(7);
            PUSH(OP_NEGATE);
        } else if CASE(Type::e_exclamation) {
            expression(7);
            PUSH(OP_NOT);
        } else if (current.type == Type::e_symbol && current.value == "set") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());
            if (ADV().value != "=")
                error("parsing error: expected '='  token: " + current.toStr());

            expression(1);
            opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
            
            return;
        } else if (current.type == Type::e_symbol && current.value == "true") {
            PUSHC(boolValue(true));
        } else if (current.type == Type::e_symbol && current.value == "false") {
            PUSHC(boolValue(false));
        } else if (current.type == Type::e_symbol && current.value == "null") {
            PUSHC(nullValue());
        } else if (current.type == Type::e_symbol && current.value == "print") { // until stl
            expression(2);
            PUSH(OP_PRINT_POP);
            return;
        } else if (current.type == Type::e_symbol && current.value == "return") { // until stl
            expression(2);
            PUSH(OP_RETURN_POP);
            return;
        } else if (current.type == Type::e_symbol && current.value == "fn") {
            if (ADV().type != Type::e_lbracket) error("parsing error: expected a '(' after 'fn'  token: " + gen.token_itr_->toStr());
            Function fn;
            while (1) {
                NEXT();
                if CASE(Type::e_symbol) {
                    if (invalidIdentifier(current.value)) error("parsing error: invalid identifier  token: " + gen.token_itr_->toStr());
                    fn.args.push_back(current.value);
                    NEXT();
                    if CASE(Type::e_comma) {
                        // nada
                    } else if CASE(Type::e_rbracket) {
                        break;
                    } else {
                        error("parsing error: expected either ')' or an identifier  token: " + gen.token_itr_->toStr());
                    }
                } else if CASE(Type::e_rbracket) {
                    break;
                } else {
                    error("parsing error: expected an identifier  token: " + gen.token_itr_->toStr());
                }
            }
            if (gen.peek_next_token().type != Type::e_lcrlbracket) {
                error("parsing error: expected a block  token: " + gen.peek_next_token().toStr());
            }
            Machine vm;
            vm.init(gen, true);
            fn.vm = vm;
            int fn_loc = heap.fn_add(fn);
            std::cout << "function stored @ " << fn_loc << ":" << std::endl;
            PUSHC(funValue(fn_loc));
        } else if (current.type == Type::e_symbol && current.value == "&") {
            expression(8);
            PUSH(OP_REFERENCE);
        } else if CASE(Type::e_mul) {
            expression(8);
            PUSH(OP_DEREFERENCE);
        } else if CASE(Type::e_colon) {
            expression(8);
            PUSH(OP_COPY);
        } else if CASE(Type::e_symbol) {
            PUSHC(idenValue(current.value));
            PUSH(OP_GET_VARIABLE);
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
        } else {
            error("parsing error: expected an expression  token: " + current.toStr());
        }
        while (p <= getPrecedence(gen.peek_next_token().type)) {
            NEXT();
            switch (current.type) {
                case Type::e_add: {
                    expression(getPrecedence(Type::e_add));
                    PUSH(OP_ADD);
                    break;
                }
                case Type::e_sub: {
                    expression(getPrecedence(Type::e_sub));
                    PUSH(OP_SUBTRACT);
                    break;
                }
                case Type::e_mul: {
                    expression(getPrecedence(Type::e_mul));
                    PUSH(OP_MULTIPLY);
                    break;
                }
                case Type::e_div: {
                    expression(getPrecedence(Type::e_div));
                    PUSH(OP_DIVIDE);
                    break;
                }
                case Type::e_mod: {
                    expression(getPrecedence(Type::e_mod));
                    PUSH(OP_MODULO);
                    break;
                }
                case Type::e_lt: {
                    expression(getPrecedence(Type::e_lt));
                    PUSH(OP_LESS);
                    break;
                }
                case Type::e_gt: {
                    expression(getPrecedence(Type::e_gt));
                    PUSH(OP_MORE);
                    break;
                }
                case Type::e_lte: {
                    expression(getPrecedence(Type::e_lte));
                    PUSH(OP_LESS_EQ);
                    break;
                }
                case Type::e_gte: {
                    expression(getPrecedence(Type::e_gte));
                    PUSH(OP_MORE_EQ);
                    break;
                }
                case Type::e_eq: {
                    if (current.value == "==") {
                        expression(getPrecedence(Type::e_eq));
                        PUSH(OP_EQUALITY);
                    } else {
                        expression(2);
                        PUSH(OP_EDIT_VARIABLE);
                    }
                    break;
                }
                case Type::e_ne: {
                    expression(getPrecedence(Type::e_ne));
                    PUSH(OP_NOT_EQUAL);
                    break;
                }

                case Type::e_lbracket: {
                    ADV();
                    PUSH(OP_CALL_FN);
                    break;
                }
            }
        }
    };
    PUSH(OP_BEGIN_SCOPE);
    while (loc < (int)gen.size()-1) {
        expression(1);
        if (fn_parsing) break;
        NEXT();
        if (current.value != ";") 
            error("parsing error: expected a semicolon  token: " + current.toStr());
        if (fn_parsing) break;
    }
    PUSH(OP_END_SCOPE);
}

#endif