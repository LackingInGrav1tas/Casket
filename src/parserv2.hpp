#ifndef PARSERV2
#define PARSERV2

#include <functional>
#include <bitset>

#include "vm.hpp"
#include "lexer.hpp"
#include "error.hpp"
#include "function.hpp"
#include "memory.hpp"

int indent = 0;

void print_debug(std::string s) {
    if (flags::debug) {
        for (int _ = 0; _ < indent; _++) {
            std::cout << "    ";
        }
        std::cout << s << std::endl;
    }
}

static int gp(TokenType t, std::string s = "") {
    // std::cout << "GP" << std::endl;
    if (s == "&") {
        return 3;
    } else if (s == "|") {
        return 2;
    } else if (s == ".") {
        return 9;
    }
    switch (t) {
        case ASSIGN        : return 1;
        case NOT_EQUALITY  : return 4;
        case EQUALITY      : return 4;
        case LESS_EQUAL    : return 5;
        case GREATER_EQUAL : return 5;
        case LESS          : return 5;
        case GREATER       : return 5;
        case PLUS          : return 6;
        case MINUS         : return 6;
        case SLASH         : return 7;
        case STAR          : return 7;
        case PERCENT       : return 7;
        case CARROT        : return 7;
        case PLUS_PLUS     : return 8;
        case MINUS_MINUS   : return 8;
        case LEFT_PAREN    : return 9;
        case LEFT_BRACE    : return 9;
        case DECIMAL       : return 9;
        default            : return 0;
    }
}

static inline int getPrecedence(TokenType t, std::string s = "") {
    int ret = gp(t, s);
    print_debug("GP(" + std::to_string(ret) + ")[" + s + "]");
    return ret;
}

static bool invalidIdentifier(std::string id) {
    return id == "set" || id == "fn" || id == "if" || id == "for" || id == "while" ||
    id == "true" || id == "false" || id == "return" || id == "null" || 
    id == "label" || id == "else" || id == "class" || id == "inst" || id == "this" || 
    id == "operator";
}

void Machine::init(Lexer &lexer, bool fn_parsing) {
    ip = 0;
    // heap.init();

    #define NEXT() \
        if (lexer.peek_next_token().type == ERROR) \
            lexer.error(lexer.peek_next_token(), "error"); \
        Token current = lexer.next_token();

    #define ERROR(token, message) lexer.error(token, message);

    #define SEMICOLON() \
        do { Token semicolon = lexer.next_token(); if (semicolon.type != SEMICOLON) { \
            ERROR(semicolon, "parsing error: expected a ';'"); \
        } } while (0)
    
    #define PUSH(arg) opcode.push_back(newOpcode(arg))
    #define PUSHC(arg) opcode.push_back(OpConstant(arg))
    #define CASE(t) (current.type == t)

    std::function<void(int)> expression;

    std::function<void(int)> exp = [&](int p)->void {
        Token current = lexer.last_token();
        print_debug("expression(" + current.value + ")");
        if CASE(LEFT_PAREN) { // group
            expression(1);
            if (lexer.peek_next_token().type != RIGHT_PAREN) {
                lexer.error(lexer.peek_next_token(), "parsing error: expected a ')'");
            }
            lexer.next_token();
        } else if (current.type == INST) {
            NEXT();
            if (current.type != T_IDENTIFIER || invalidIdentifier(current.value))
                ERROR(current, "parsing error: expected a valid class name");
            if (lexer.next_token().type == LEFT_PAREN) {
                int i = 0;
                if (lexer.peek_next_token().type != RIGHT_PAREN) {
                    while (1) {
                        i++;
                        if (lexer.peek_next_token().type != T_IDENTIFIER || invalidIdentifier(lexer.peek_next_token().value))
                            ERROR(lexer.peek_next_token(), "parsing error: expected a valid identifier");
                        PUSHC(idenValue(lexer.peek_next_token().value));
                        lexer.next_token();
                        if (lexer.peek_next_token().value != "=") {
                            ERROR(lexer.peek_next_token(), "parsing error: expected '='");
                        }
                        lexer.next_token();
                        expression(2);
                        NEXT();
                        if CASE(COMMA) {
                            // nada
                        } else if CASE(RIGHT_PAREN) {
                            break;
                        }
                    }
                } else {
                    lexer.next_token();
                }
                opcode.push_back(InstanceOpcode(current.value, i));
            } else {
                ERROR(lexer.last_token(), "parsing error: expected a ')'");
            }

        } else if CASE(MINUS) {
            expression(7);
            PUSH(OP_NEGATE);
        } else if CASE(EXCLAMATION_MARK) {
            expression(7);
            PUSH(OP_NOT);
        } else if (current.type == TRUE) {
            PUSHC(boolValue(true));
        } else if (current.type == FALSE) {
            PUSHC(boolValue(false));
        } else if (current.type == T_NIL) {
            PUSHC(nullValue());
        } else if (current.type == FN) {
            print_debug("fn");
            if (lexer.next_token().type != LEFT_PAREN)
                ERROR(lexer.last_token(), "parsing error: expected a '(' after 'fn' token");
            Function fn;
            while (1) {
                NEXT();
                print_debug("paren parsing: " + current.value + " " + std::to_string(current.type));
                if CASE(T_IDENTIFIER) {
                    print_debug("case identifier");
                    if (invalidIdentifier(current.value))
                        ERROR(lexer.last_token(), "parsing error: invalid identifier");
                    fn.args.push_back(current.value);
                    NEXT();
                    if CASE(COMMA) {
                        // nada
                    } else if CASE(RIGHT_PAREN) {
                        break;
                    } else {
                        ERROR(lexer.last_token(), "parsing error: expected either ')' or an identifier");
                    }
                } else if CASE(RIGHT_PAREN) {
                    print_debug("case r paren");
                    break;
                } else {
                    ERROR(current, "parsing error: expected an identifier");
                }
            }
            if (lexer.peek_next_token().type != LEFT_BRACKET) {
                ERROR(lexer.last_token(), "parsing error: expected a block token");
            }
            Machine vm;
            // std::cout << "next: " << lexer.peek_next_token().toStr() << std::endl;
            vm.init(lexer, true);
            // std::cout << "done with it!" << std::endl;
            // vm.disassemble();
            fn.vm = vm;
            int fn_loc = heap.fn_add(fn);
            // std::cout << "function stored @ " << fn_loc << ":" << std::endl;
            print_debug(std::to_string(fn.args.size()));
            PUSHC(funValue(fn_loc));
            print_debug("FN DONE - " + lexer.peek_next_token().value);
        } else if (current.type == BIT_AND) {
            expression(8);
            PUSH(OP_REFERENCE);
        } else if CASE(STAR) {
            expression(8);
            PUSH(OP_DEREFERENCE);
        } else if CASE(COLON) {
            expression(8);
            PUSH(OP_COPY);
        } else if CASE(T_IDENTIFIER) {
            if (invalidIdentifier(current.value) && current.value != "this")
                ERROR(current, "parsing error: expected a valid identifier");
            PUSHC(idenValue(current.value));
            PUSH(OP_GET_VARIABLE);
        } else if CASE(T_BYTE) {
            PUSHC( byteValue( std::bitset<8>(current.value).to_ulong() ) );

        } else if CASE(T_NUMBER) {
            try {
                int is_float = false;
                for (int i = 0; i < current.value.length(); i++) {
                    if (current.value[i] == '.') {
                        is_float = true;
                        break;
                    }
                }
                if (is_float) PUSHC(floatValue(std::stof(current.value)));
                else {
                    if (lexer.peek_next_token().value == "byte") { // remove when as conversion implemented
                        lexer.next_token();
                        PUSHC(byteValue((unsigned char) std::stoi(current.value)));
                    } else {
                        PUSHC(intValue(std::stoi(current.value)));
                    }
                }
            } catch(...) {
                ERROR(current, "parsing error: couldn't parse number");
            }
        } else if CASE(T_STRING) {
            PUSHC(strValue(current.value));
        } else if CASE(LEFT_BRACE) {
            int i = 0;
            while (lexer.peek_next_token().type != RIGHT_BRACE) {
                expression(2);
                // std::cout << lexer.peek_next_token().value << std::endl;
                if (lexer.peek_next_token().value == ",") lexer.next_token();
                i++;
            }
            lexer.next_token();
            opcode.push_back(ListOpcode(i));
        } else {
            ERROR(current, "parsing error: expected an expression");
        }
        // std::cout << "peeking for precedence: " << lexer.peek_next_token().toStr() << std::endl;
        while (p <= getPrecedence(lexer.peek_next_token().type, lexer.peek_next_token().value)) {
            NEXT();
            // std::cout << current.value << std::endl;
            if (current.value == ".") {
                NEXT();
                // std::cout << "checking" << std::endl;
                if (current.type != T_IDENTIFIER || invalidIdentifier(current.value))
                    ERROR(current, "parsing error: expected an identifier");
                // std::cout << "PUSHC" << std::endl;
                PUSHC(idenValue(current.value));
                // std::cout << "PUSH" << std::endl;
                PUSH(OP_GET_MEMBER);
                // std::cout << "past" << std::endl;
            } else if (current.value == "&") {
                if (lexer.peek_next_token().value == "&") {
                    lexer.next_token();
                    expression(2);
                    PUSH(OP_AND);
                } else {
                    ERROR(lexer.peek_next_token(), "parsing error: expected a ';D'");
                }
            } else if (current.value == "|") {
                if (lexer.peek_next_token().value == "|") {
                    lexer.next_token();
                    expression(1);
                    PUSH(OP_OR);
                } else {
                    ERROR(lexer.peek_next_token(), "parsing error: expected a ';('");
                }
            } else {
                switch (current.type) {
                    case PLUS: {
                        expression(getPrecedence(PLUS));
                        PUSH(OP_ADD);
                        break;
                    }
                    case PLUS_PLUS: {
                        lexer.next_token();
                        PUSH(OP_INCREMENT);
                        break;
                    }
                    case MINUS: {
                        expression(getPrecedence(MINUS));
                        PUSH(OP_SUBTRACT);
                        break;
                    }
                    case MINUS_MINUS: {
                        lexer.next_token();
                        PUSH(OP_DECREMENT);
                        break;
                    }
                    case STAR: {
                        expression(getPrecedence(STAR));
                        PUSH(OP_MULTIPLY);
                        break;
                    }
                    case SLASH: {
                        expression(getPrecedence(SLASH));
                        PUSH(OP_DIVIDE);
                        break;
                    }
                    case PERCENT: {
                        expression(getPrecedence(PERCENT));
                        PUSH(OP_MODULO);
                        break;
                    }
                    case LESS: {
                        expression(getPrecedence(LESS));
                        PUSH(OP_LESS);
                        break;
                    }
                    case GREATER: {
                        expression(getPrecedence(GREATER));
                        PUSH(OP_MORE);
                        break;
                    }
                    case LESS_EQUAL: {
                        expression(getPrecedence(LESS_EQUAL));
                        PUSH(OP_LESS_EQ);
                        break;
                    }
                    case GREATER_EQUAL: {
                        expression(getPrecedence(GREATER_EQUAL));
                        PUSH(OP_MORE_EQ);
                        break;
                    }
                    case EQUALITY: {
                        expression(getPrecedence(EQUALITY));
                        PUSH(OP_EQUALITY);
                        break;
                    }
                    case ASSIGN: {
                        expression(2);
                        PUSH(OP_EDIT_VARIABLE);
                        break;
                    }
                    case NOT_EQUALITY: {
                        expression(getPrecedence(NOT_EQUALITY));
                        PUSH(OP_NOT_EQUAL);
                        break;
                    }

                    case LEFT_PAREN: {
                        int i = 0;
                        if (lexer.peek_next_token().type != RIGHT_PAREN) {
                            while (1) {
                                i++;
                                expression(2);
                                NEXT();
                                if (current.type == COMMA) {
                                    // nada
                                } else if (current.type == RIGHT_PAREN) {
                                    break;
                                }
                            }
                        } else {
                            lexer.next_token();
                        }
                        opcode.push_back(callOpcode(i));
                        break;
                    }

                    case LEFT_BRACE: {
                        expression(2);
                        NEXT();
                        if (current.type != RIGHT_BRACE) {
                            ERROR(current, "parsing error: expected a ']'");
                        }
                        PUSH(OP_INDEX);
                        break;
                    }

                    case DECIMAL: {
                        NEXT();
                        if (current.type != T_IDENTIFIER || invalidIdentifier(current.value))
                            ERROR(current, "parsing error: expected a valid identifier");
                        PUSHC(idenValue(current.value));
                        PUSH(OP_GET_MEMBER);
                        break;
                    }
                    default: {
                        std::cerr << "SHOULD BE UNREACHABLE (" << current.type << ")" << std::endl;
                        exit(1);
                    }
                }
            }
        }
    };

    expression = [&](int p)->void {
        lexer.next_token();
        exp(p);
    };

    std::function<void()> declaration = [&]() -> void {
        Token check = lexer.next_token();
        print_debug("declaration(" + check.value + ") {");
        indent++;
        if (check.value == "set") {
            NEXT();
            std::string id = current.value;
            if (current.type != T_IDENTIFIER || invalidIdentifier(current.value) )
                ERROR(current, "parsing error: invalid identifier");
            std::string nxt = lexer.next_token().value;
            if (nxt != "=") {
                if (nxt == ";") {
                    PUSHC(nullValue());
                    opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
                    return;
                } else 
                    ERROR(lexer.last_token(), "parsing error: expected a '='");
            }
                

            expression(1);

            SEMICOLON();
            
            opcode.push_back(spOpcode(OP_SET_VARIABLE, id));
        } else if (check.value == "if") {
            if (lexer.next_token().type != LEFT_PAREN)
                ERROR(lexer.peek_next_token(), "parsing error: expected a '('");
            
            expression(2);

            if (lexer.next_token().value != ")")
                ERROR(lexer.last_token(), "parsing error: expected a ')'");

            PUSH(OP_ERROR);
            int size = opcode.size();

            declaration();

            //std::cout << "\nvalue = " << lexer.last_token().value << std::endl;
            if (lexer.last_token().value == "else") {
                lexer.next_token();
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
            if (current.type != LEFT_PAREN)
                ERROR(current, "parsing error: expected a '('");
            
            int at_condition_size = opcode.size();

            expression(2);

            Token end_br = lexer.next_token();
            if (end_br.type != RIGHT_PAREN)
                ERROR(end_br, "parsing error: expected a ')'");

            int size = opcode.size(); // points to below jump
            PUSH(OP_ERROR);
            
            declaration();

            opcode.push_back(jumpOpcode(OP_JUMP, at_condition_size-opcode.size()-1));
            
            opcode[size] = jumpOpcode(OP_JUMP_FALSE, opcode.size()-size-1);

            PUSH(OP_END_SCOPE);
        } else if (check.value == "for") {
            if (lexer.next_token().type != LEFT_PAREN) ERROR(lexer.last_token(), "parsing error: expected a '('");
            declaration(); // for (; 
            PUSH(OP_BEGIN_SCOPE);

            int at_condition_size = opcode.size(); // condition

            expression(2);

            SEMICOLON();

            int size = opcode.size(); // points to below jump
            PUSH(OP_ERROR);
            
            int iter_pre_size = opcode.size();
            expression(1); // iter
            int iter_post_size = opcode.size();

            Token bracket = lexer.next_token();
            if (bracket.type != RIGHT_PAREN) ERROR(bracket, "parsing error: expected a ')'");

            declaration();

            for (int i = iter_pre_size; i < iter_post_size; i++) {
                opcode.push_back(opcode[iter_pre_size]);
                opcode.erase(opcode.begin()+iter_pre_size);
            }

            opcode.push_back(jumpOpcode(OP_JUMP, at_condition_size-opcode.size()-1));
            
            opcode[size] = jumpOpcode(OP_JUMP_FALSE, opcode.size()-size-1);

            PUSH(OP_END_SCOPE);

        } else if (check.value == "class") {
            NEXT();
            if (current.type != T_IDENTIFIER || invalidIdentifier(current.value)) {
                ERROR(current, "parsing error: expected a valid identifier");
            }
            std::string name = current.value;
            if (lexer.next_token().type != LEFT_BRACKET) {
                ERROR(lexer.peek_next_token(), "parsing error: expected a valid identifier")
            }

            int i = 0;
            while (1) {
                if (lexer.peek_next_token().type == RIGHT_BRACKET) {
                    break;
                } else {
                    i++;
                    Token tname = lexer.next_token();

                    if (tname.value == "operator") {
                        Token ntk = lexer.next_token();
                        print_debug("method(" + ntk.value + ")");
                        #define EQUALS(op) op == ntk.value
                        if (EQUALS("+")) {
                            if (lexer.peek_next_token().value == "+") {
                                PUSHC(idenValue("++"));
                                lexer.next_token();
                            } else {
                                PUSHC(idenValue("+"));
                            }
                        } else if (EQUALS("-")) {
                            if (lexer.peek_next_token().value == "-") {
                                PUSHC(idenValue("--"));
                                lexer.next_token();
                            } else {
                                PUSHC(idenValue("-"));
                            }
                        } else if (EQUALS("=")) {
                            if (lexer.peek_next_token().value == "=") {
                                PUSHC(idenValue("=="));
                                lexer.next_token();
                            } else {
                                ERROR(ntk, "parsing error: '=' is not bindable");
                            }
                        } else if (EQUALS("!")) {
                            if (lexer.peek_next_token().value == "=") {
                                PUSHC(idenValue("!="));
                                lexer.next_token();
                            } else {
                                PUSHC(idenValue("!"));
                            }
                        } else if (EQUALS("*") || EQUALS("/") || EQUALS("%") || EQUALS("[") || EQUALS(">")
                                || EQUALS("<") || EQUALS(">=") || EQUALS("<=")) {
                            PUSHC(idenValue(ntk.value));
                        } else if (EQUALS("prefix_negate")) {
                            PUSHC(idenValue("prefix-"));
                        } else {
                            ERROR(ntk, "parsing error: expected a bindable operator");
                        }
                        #undef EQUALS
                        // lexer.next_token();
                    } else if (tname.type != T_IDENTIFIER || invalidIdentifier(tname.value)) {
                        ERROR(tname, "parsing error: expected an identifier");
                    } else {
                        print_debug("method(" + tname.value + ")");
                        PUSHC(idenValue(tname.value));
                    }
                    NEXT();
                    if CASE(COLON) {
                        expression(1);
                        NEXT();
                        if (current.value == ";") {
                            // nada
                        } else if CASE(RIGHT_PAREN) {
                            break;
                        }
                    } else if (current.value != ";") {
                        ERROR(current, "parsing error: expected either ';' or ':'");
                    } else {
                        PUSHC(nullValue());
                    }
                }
            }

            opcode.push_back(DeclClassOpcode(name, i));

            lexer.next_token();
        } else if (check.value == "{") {
            opcode.push_back(newOpcode(OP_BEGIN_SCOPE));
            while (!lexer.done()) {
                declaration();
                if (lexer.peek_next_token().value == "}") {
                    lexer.next_token();
                    break;
                }
            }
            // if (lexer.finished()) error("parsing error: unclosed bracket");
            opcode.push_back(newOpcode(OP_END_SCOPE));
        } else if (check.value == "label") {
            NEXT();
            std::string id = current.value;
            if (current.type != T_IDENTIFIER || invalidIdentifier(current.value) )
                ERROR(current, "parsing error: invalid identifier");

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_LABEL;
            opcode.push_back(op);
            SEMICOLON();
        } else if (check.value == "goto") {
            NEXT();
            std::string id = current.value;
            if (current.type != T_IDENTIFIER || invalidIdentifier(current.value) )
                ERROR(current, "parsing error: invalid identifier");

            OpcodeObject op;
            op.lexeme = id;
            op.op = OP_GOTO_LABEL;
            opcode.push_back(op);
            SEMICOLON();
        } else if (check.value == "return") {
            expression(2);
            SEMICOLON();
            PUSH(OP_RETURN_POP);
        } else if (check.value == ";") {
            // nada
        } else {
            exp(1);
            SEMICOLON();
        }
        indent--;
        print_debug("}");
    };

    opcode.push_back(newOpcode(OP_BEGIN_SCOPE));
    while (!lexer.done()) {
        declaration();
        if (fn_parsing) break;
    }
    opcode.push_back(newOpcode(OP_END_SCOPE));
}

#endif