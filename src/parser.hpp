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
    bool exempt = false;
    static int loc = -1;
    heap.init();

    // <stl>

    ClassTemplate str_template;
    str_template.members["str"] = nullValue();
    Function string_to_str;
    string_to_str.vm.opcode = {
        newOpcode(OP_BEGIN_SCOPE),
        OpConstant(idenValue("this")),
        newOpcode(OP_GET_VARIABLE),
        OpConstant(idenValue("str")),
        newOpcode(OP_GET_MEMBER),
        newOpcode(OP_RETURN_POP),
        newOpcode(OP_END_SCOPE),
    };
    str_template.members["to_string"] = funValue(heap.fn_add(string_to_str));
    templates.push_back(std::map<std::string, ClassTemplate>());
    templates.back()["String"] = str_template;

    // </stl>

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
        std::cout << "expression(" << p << ")" << std::endl;
        NEXT();
        if CASE(Type::e_lbracket) { // group
            expression(1);
            if (gen.token_itr_->type != Type::e_rbracket) {
                error("parsing error: expected a ')'  token: " + PEEK().toStr());
            }
            ADV();
        } else if CASE(Type::e_lcrlbracket) {
            std::cout << "{" << std::endl;
            PUSH(OP_BEGIN_SCOPE);
            while (1) {
                expression(1);
                auto v = ADV();
                if (v.value != ";" && !exempt) error("parsing error: expected a semicolon  token: " + v.toStr());
                exempt = false;
                std::cout << "peek(): " << PEEK().toStr() << std::endl;
                if (gen.peek_next_token().type == Type::e_rcrlbracket) break;
            }
            PUSH(OP_END_SCOPE);
            ADV();
            exempt = true;
            return;
        } else if (current.type == Type::e_symbol && current.value == "class") {
            std::cout << "class" << std::endl;
            NEXT();
            if (current.type != Type::e_symbol || invalidIdentifier(current.value)) {
                error("parsing error: expected a valid identifier  token: " + current.toStr());
            }
            std::string name = current.value;
            if (ADV().type != Type::e_lcrlbracket) {
                error("parsing error: expected a valid identifier  token: " + PEEK().toStr());
            }

            int i = 0;
            while (1) {
                if (PEEK().type == Type::e_rcrlbracket) {
                    break;
                } else {
                    i++;
                    Token tname = ADV();
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

            exempt = true;

            ADV();

            return;
        } else if (current.type == Type::e_symbol && current.value == "inst") { // until stl
            std::cout << "inst" << std::endl;
            NEXT();
            if (current.type != Type::e_symbol || invalidIdentifier(current.value))
                error("parsing error: expected a valid class name  token: " + current.toStr());
            if (ADV().type == Type::e_lbracket) {
                int i = 0;
                if (gen.peek_next_token().type != Type::e_rbracket) {
                    while (1) {
                        i++;
                        if (PEEK().type != Type::e_symbol || invalidIdentifier(PEEK().value))
                            error("parsing error: expected a valid identifer  token: " + PEEK().toStr());
                        PUSHC(idenValue(PEEK().value));
                        ADV();
                        if (ADV().value != "=") {
                            std::cout << "'" << gen.token_itr_->value << "'" << std::endl;
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
                    ADV();
                }
                opcode.push_back(InstanceOpcode(current.value, i));
            } else {
                error("parsing error: expected a ')'  token: " + gen.token_itr_->toStr());
            }

        } else if (current.type == Type::e_symbol && current.value == "if") {
            std::cout << "if" << std::endl;
            if (gen.peek_next_token().type != Type::e_lbracket) error("parsing error: expected a '('  token: " + gen.peek_next_token().toStr());
            // it will find a grouping and take care of the matching ')'
            expression(2);

            PUSH(OP_ERROR);
            int size = opcode.size();

            expression(1);

            //std::cout << "\nvalue = " << gen.token_itr_->value << std::endl;
            if (gen.token_itr_->value == "else") {
                ADV();
                PUSH(OP_ERROR);
                int elsesize = opcode.size();

                opcode[size-1] = jumpOpcode(OP_JUMP_FALSE, opcode.size() - size);

                expression(1);

                opcode[elsesize-1] = jumpOpcode(OP_JUMP, opcode.size() - elsesize);
            } else {
                opcode[size-1] = jumpOpcode(OP_JUMP_FALSE, opcode.size() - size);
            }

            return;
        } else if (current.type == Type::e_symbol && current.value == "while") {
            if (gen.peek_next_token().type != Type::e_lbracket) error("parsing error: expected a '('  token: " + gen.peek_next_token().toStr());
            // it will find a grouping and take care of the matching ')'
            expression(2);


        } else if CASE(Type::e_sub) {
            expression(7);
            PUSH(OP_NEGATE);
        } else if CASE(Type::e_exclamation) {
            expression(7);
            PUSH(OP_NOT);
        } else if (current.type == Type::e_symbol && current.value == "set") {
            std::cout << "set" << std::endl;
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());
            if (ADV().value != "=")
                error("parsing error: expected '='  token: " + gen.token_itr_->toStr());

            expression(1);
            opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
            
            return;
        } else if (current.type == Type::e_symbol && current.value == "label") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_LABEL;
            opcode.push_back(op);
            return;
        } else if (current.type == Type::e_symbol && current.value == "goto") {
            NEXT();
            std::string id = current.value;
            if (current.type != Type::e_symbol || invalidIdentifier(current.value) )
                error("parsing error: invalid identifier  token: " + current.toStr());

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_GOTO_LABEL;
            opcode.push_back(op);
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
            std::cout << "fn" << std::endl;
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
            std::cout << "done with it!" << std::endl;
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
        } else {
            error("parsing error: expected an expression  token: " + current.toStr());
        }
        while (p <= getPrecedence(gen.peek_next_token().type)) {
            NEXT();
            if (current.value == ".") {
                NEXT();
                if (current.type != Type::e_symbol || invalidIdentifier(current.value))
                    error("parsing error: expected an identifier");
                PUSHC(idenValue(current.value));
                PUSH(OP_GET_MEMBER);
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
                            std::cout << "==" << std::endl;
                            expression(getPrecedence(Type::e_eq));
                            PUSH(OP_EQUALITY);
                        } else {
                            std::cout << "=" << std::endl;
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
                                if CASE(Type::e_comma) {
                                    // nada
                                } else if CASE(Type::e_rbracket) {
                                    break;
                                }
                            }
                        } else {
                            ADV();
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
    PUSH(OP_BEGIN_SCOPE);
    while (loc < (int)gen.size()-1) {
        expression(1);
        if (fn_parsing) break;
        if (!exempt) {
            NEXT();
            if (current.value != ";") 
                error("parsing error: expected a semicolon   token: " + current.toStr());
            if (fn_parsing) break;
        } else
            exempt = false;
        std::cout << ";" << std::endl;
    }
    PUSH(OP_END_SCOPE);
}

#endif