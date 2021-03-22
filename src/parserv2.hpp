#ifndef PARSERV2
#define PARSERV2

#include <functional>

#include "vm.hpp"
#include "lexertk.hpp"
#include "error.hpp"
#include "function.hpp"
#include "memory.hpp"

typedef lexertk::generator Generator;
typedef lexertk::token Token;
typedef lexertk::token::token_type Type;

static int getPrecedence(Type t, std::string s = "") {
    if (s == "&") {
        return 3;
    } else if (s == "|") {
        return 2;
    }
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
        case Type::e_lsqrbracket : return 9;
        case Type::e_lcrlbracket : return 9;
        case Type::e_number      : return 9; // inst.member
        default                  : return 0;
    }
}

static bool invalidIdentifier(std::string id) {
    return id == "set" || id == "fn" || id == "if" || id == "for" || id == "while" ||
    id == "true" || id == "false" || id == "print" || id == "return" || id == "null" || 
    id == "label" || id == "else" || id == "class" || id == "inst" || id == "this";
}

void Machine::init(Generator &gen, bool fn_parsing) {
    ip = 0;
    heap.init();

    #define NEXT() \
        if (gen.peek_next_token().is_error()) \
            error("parsing error: " + gen.peek_next_token().toStr()); \
        Token current = gen.next_token();

    #define SEMICOLON() \
        if (gen.next_token().value != ";") { \
            error("parsing error: expected a ';'  token: " + (gen.token_itr_-1)->toStr()); \
        }
    
    #define PUSH(arg) opcode.push_back(newOpcode(arg))
    #define PUSHC(arg) opcode.push_back(OpConstant(arg))
    #define CASE(t) (current.type == t)

    std::function<void(int)> expression;

    std::function<void(int)> exp = [&](int p)->void {
        Token current = *(gen.token_itr_-1);
        // std::cout << "expression(" << current.toStr() << ")" << std::endl;
        if CASE(Type::e_lbracket) { // group
            expression(1);
            if (gen.token_itr_->type != Type::e_rbracket) {
                error("parsing error: expected a ')'  token: " + gen.peek_next_token().toStr());
            }
            gen.next_token();
        } else if (current.type == Type::e_symbol && current.value == "inst") { // until stl
            NEXT();
            if (current.type != Type::e_symbol || invalidIdentifier(current.value))
                error("parsing error: expected a valid class name  token: " + current.toStr());
            if (gen.next_token().type == Type::e_lbracket) {
                int i = 0;
                if (gen.peek_next_token().type != Type::e_rbracket) {
                    while (1) {
                        i++;
                        if (gen.peek_next_token().type != Type::e_symbol || invalidIdentifier(gen.peek_next_token().value))
                            error("parsing error: expected a valid identifer  token: " + gen.peek_next_token().toStr());
                        PUSHC(idenValue(gen.peek_next_token().value));
                        gen.next_token();
                        if (gen.next_token().value != "=") {
                            error("parsing error: expected '='  token: " + gen.token_itr_->toStr());
                        }
                        expression(2);
                        NEXT();
                        if CASE(Type::e_comma) {
                            // nada
                        } else if CASE(Type::e_rbracket) {
                            break;
                        }
                    }
                } else {
                    gen.next_token();
                }
                opcode.push_back(InstanceOpcode(current.value, i));
            } else {
                error("parsing error: expected a ')'  token: " + gen.token_itr_->toStr());
            }

        } else if CASE(Type::e_sub) {
            expression(7);
            PUSH(OP_NEGATE);
        } else if CASE(Type::e_exclamation) {
            expression(7);
            PUSH(OP_NOT);
        } else if (current.type == Type::e_symbol && current.value == "true") {
            PUSHC(boolValue(true));
        } else if (current.type == Type::e_symbol && current.value == "false") {
            PUSHC(boolValue(false));
        } else if (current.type == Type::e_symbol && current.value == "null") {
            PUSHC(nullValue());
        } else if (current.type == Type::e_symbol && current.value == "fn") {
            // std::cout << "fn" << std::endl;
            if (gen.next_token().type != Type::e_lbracket) error("parsing error: expected a '(' after 'fn'  token: " + gen.token_itr_->toStr());
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
            // std::cout << "next: " << gen.peek_next_token().toStr() << std::endl;
            vm.init(gen, true);
            // std::cout << "done with it!" << std::endl;
            // vm.disassemble();
            fn.vm = vm;
            int fn_loc = heap.fn_add(fn);
            // std::cout << "function stored @ " << fn_loc << ":" << std::endl;
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
            if (invalidIdentifier(current.value) && current.value != "this")
                error("parsing error: expected a valid identifier  token: " + current.toStr());
            PUSHC(idenValue(current.value));
            PUSH(OP_GET_VARIABLE);
        } else if CASE(Type::e_number) {
            try {
                int is_float = false;
                for (int i = 0; i < current.value.length(); i++) {
                    if (current.value[i] == '.') {
                        is_float = true;
                        break;
                    }
                }
                if (is_float) PUSHC(floatValue(std::stof(current.value)));
                else PUSHC(intValue(std::stoi(current.value)));
            } catch(...) {
                error("parsing error: couldn't parse number  token: " + current.toStr());
            }
        } else if CASE(Type::e_string) {
            PUSHC(strValue(current.value));
        } else if CASE(Type::e_lsqrbracket) {
            int i = 0;
            while (gen.peek_next_token().type != Type::e_rsqrbracket) {
                expression(2);
                std::cout << gen.peek_next_token().value << std::endl;
                if (gen.peek_next_token().value == ",") gen.next_token();
                i++;
            }
            gen.next_token();
            opcode.push_back(ListOpcode(i));
        } else {
            error("parsing error: expected an expression  token: " + current.toStr());
        }
        // std::cout << "peeking for precedence: " << gen.peek_next_token().toStr() << std::endl;
        while (p <= getPrecedence(gen.peek_next_token().type, gen.peek_next_token().value)) {
            NEXT();
            if (current.value == ".") {
                NEXT();
                if (current.type != Type::e_symbol || invalidIdentifier(current.value))
                    error("parsing error: expected an identifier");
                PUSHC(idenValue(current.value));
                PUSH(OP_GET_MEMBER);
            } else if (current.value == "&") {
                if (gen.peek_next_token().value == "&") {
                    gen.next_token();
                    expression(2);
                    PUSH(OP_AND);
                } else {
                    error("parsing error: expected a ';'  token: " + gen.peek_next_token().toStr());
                }
            } else if (current.value == "|") {
                if (gen.peek_next_token().value == "|") {
                    gen.next_token();
                    expression(1);
                    PUSH(OP_OR);
                } else {
                    error("parsing error: expected a ';'  token: " + gen.peek_next_token().toStr());
                }
            } else {
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
                        int i = 0;
                        if (gen.peek_next_token().type != Type::e_rbracket) {
                            while (1) {
                                i++;
                                expression(2);
                                NEXT();
                                if (current.type == Type::e_comma) {
                                    // nada
                                } else if (current.type == Type::e_rbracket) {
                                    break;
                                }
                            }
                        } else {
                            gen.next_token();
                        }
                        opcode.push_back(callOpcode(i));
                        break;
                    }

                    case Type::e_lsqrbracket: {
                        expression(2);
                        NEXT();
                        if (current.type != Type::e_rsqrbracket) {
                            error("parsing error: expected a ']' " + current.toStr());
                        }
                        PUSH(OP_INDEX);
                        break;
                    }

                    case Type::e_number: {
                        if (current.value == ".") {
                            NEXT();
                            if (current.type != Type::e_symbol || invalidIdentifier(current.value))
                                error("parsing error: expected an identifier  token: " + current.toStr());
                            PUSHC(idenValue(current.value));
                            PUSH(OP_GET_MEMBER);
                        } else {
                            error("parsing error: expected a semicolon -- 390.");
                        }
                    }
                }
            }
        }
    };

    expression = [&](int p)->void {
        gen.next_token();
        exp(p);
    };

    std::function<void()> declaration = [&]() -> void {
        Token check = gen.next_token();
        // std::cout << "declaration(" << check.toStr() << ")" << std::endl;
        if (check.value == "set") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());
            std::string nxt = gen.next_token().value;
            if (nxt != "=") {
                if (nxt == ";") {
                    PUSHC(nullValue());
                    opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
                    return;
                } else 
                    error("parsing error: expected '='  token: " + gen.token_itr_->toStr());
            }
                

            expression(1);

            SEMICOLON();
            
            opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
        } else if (check.value == "if") {
            if (gen.next_token().type != Type::e_lbracket) error("parsing error: expected a '('  token: " + gen.peek_next_token().toStr());
            
            expression(2);

            if (gen.next_token().value != ")") error("parsing error: expected a ')'  token: " + gen.token_itr_->toStr());

            PUSH(OP_ERROR);
            int size = opcode.size();

            declaration();

            //std::cout << "\nvalue = " << gen.token_itr_->value << std::endl;
            if (gen.token_itr_->value == "else") {
                gen.next_token();
                PUSH(OP_ERROR);
                int elsesize = opcode.size();

                opcode[size-1] = jumpOpcode(OP_JUMP_FALSE, opcode.size() - size);

                declaration();

                opcode[elsesize-1] = jumpOpcode(OP_JUMP, opcode.size() - elsesize);
            } else {
                opcode[size-1] = jumpOpcode(OP_JUMP_FALSE, opcode.size() - size);
            }
        } else if (check.value == "while") {
            PUSH(OP_BEGIN_SCOPE);
            NEXT();
            if (current.type != Type::e_lbracket) error("parsing error: expected '('  token: " + current.toStr());
            
            int at_condition_size = opcode.size();

            expression(2);

            Token end_br = gen.next_token();
            if (end_br.type != Type::e_rbracket) error("parsing error: expected ')'  token: " + end_br.toStr());

            int size = opcode.size(); // points to below jump
            PUSH(OP_ERROR);
            
            declaration();

            opcode.push_back(jumpOpcode(OP_JUMP, at_condition_size-opcode.size()-1));
            
            opcode[size] = jumpOpcode(OP_JUMP_FALSE, opcode.size()-size-1);

            PUSH(OP_END_SCOPE);

        } else if (check.value == "class") {
            NEXT();
            if (current.type != Type::e_symbol || invalidIdentifier(current.value)) {
                error("parsing error: expected a valid identifier  token: " + current.toStr());
            }
            std::string name = current.value;
            if (gen.next_token().type != Type::e_lcrlbracket) {
                error("parsing error: expected a valid identifier  token: " + gen.peek_next_token().toStr());
            }

            int i = 0;
            while (1) {
                if (gen.peek_next_token().type == Type::e_rcrlbracket) {
                    break;
                } else {
                    i++;
                    Token tname = gen.next_token();
                    if (tname.type != Type::e_symbol || invalidIdentifier(tname.value)) {
                        error("parsing error: expected an identifier  !!  token: " + tname.toStr());
                    }
                    PUSHC(idenValue(tname.value));
                    NEXT();
                    if CASE(Type::e_colon) {
                        expression(1);
                        NEXT();
                        if (current.value == ";") {
                            // nada
                        } else if CASE(Type::e_rbracket) {
                            break;
                        }
                    } else if (current.value != ";") {
                        error("parsing error: expected either ';' or ':'  token: " + current.toStr());
                    } else {
                        PUSHC(nullValue());
                    }
                }
            }

            opcode.push_back(DeclClassOpcode(name, i));

            // exempt = true;

            gen.next_token();
        } else if (check.value == "{") {
            opcode.push_back(newOpcode(OP_BEGIN_SCOPE));
            while (!gen.finished()) {
                declaration();
                if (gen.peek_next_token().value == "}") {
                    gen.next_token();
                    break;
                }
            }
            // if (gen.finished()) error("parsing error: unclosed bracket");
            opcode.push_back(newOpcode(OP_END_SCOPE));
        } else if (check.value == "label") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_LABEL;
            opcode.push_back(op);
            SEMICOLON();
        } else if (check.value == "goto") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_GOTO_LABEL;
            opcode.push_back(op);
            SEMICOLON();
        } else if (check.value == "print") { // until stl
            expression(2);
            SEMICOLON();
            PUSH(OP_PRINT_POP);
        } else if (check.value == "return") { // until stl
            expression(2);
            SEMICOLON();
            PUSH(OP_RETURN_POP);
        } else {
            exp(1);
            SEMICOLON();
        }
    };

    opcode.push_back(newOpcode(OP_BEGIN_SCOPE));
    while (!gen.finished()) {
        declaration();
        if (fn_parsing) break;
    }
    opcode.push_back(newOpcode(OP_END_SCOPE));
}

#endif