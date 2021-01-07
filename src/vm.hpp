#ifndef VM_HPP
#define VM_HPP

#include "opcode.hpp"
#include "memory.hpp"
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
    
    void init(lexertk::generator gen);

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
            }
            std::cout << std::endl;
        }
    }

    Value run() {
        // TODO: when a fn returns an object, reset it's box location unless it returns &
        #define TOP() Value top = stack.top(); stack.pop()
        #define SIDES() Value rhs = stack.top(); stack.pop(); Value lhs = stack.top(); stack.pop()
        #define INSTRUCTION opcode[ip]
        #define OP INSTRUCTION.op

        #define BASIC_OPERATION(operator) \
do { \
    SIDES(); \
    if (lhs.type == FLOAT) { \
        if (rhs.type == FLOAT) { \
            stack.push(floatValue( lhs.getFloat() operator rhs.getFloat() )); \
        } else if (rhs.type == INTIGER) { \
            stack.push(floatValue( lhs.getFloat() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else if (lhs.type == INTIGER) { \
        if (rhs.type == FLOAT) { \
            stack.push(floatValue( lhs.getInt() operator rhs.getFloat() )); \
        } else if (rhs.type == INTIGER) { \
            stack.push(intValue( lhs.getInt() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else { \
        lhs.error("invalid type for operation"); \
    } \
} while (0)

        for (; ip < opcode.size(); ip++) {
            // std::cout << "#: " << ip << "  type: " << OP << std::endl;
            if (OP == OP_CONSTANT) {
                stack.push(INSTRUCTION.value);

            } else if (OP == OP_SET_VARIABLE) {
                // stack order: new value (ID comes with op)
                // notation: set var = value
                TOP();
                scopes.back()[INSTRUCTION.lexeme] = heap.add(top);
            } else if (OP == OP_GET_VARIABLE) {
                TOP();
                bool found = false;
                for (int s = scopes.size()-1; s >= 0; s--) {
                    auto it = scopes[s].find(top.getIden());
                    if (it != scopes[s].end()) {
                        found = true;
                        stack.push(heap.get(it->second));
                        break;
                    }
                }
                if (!found) top.error("identifier is not in scope");
            } else if (OP == OP_EDIT_VARIABLE) {
                // stack order: new value, pointer
                // notation: ptr = new_value
                SIDES();
                heap.change(lhs.getBoxLoc(), rhs);

            } else if (OP == OP_REFERENCE) {
                // stack order: identifier
                // notation: &<id-expr>
                TOP();
                /*bool found = false;
                for (int s = scopes.size()-1; s >= 0; s--) {
                    auto it = scopes[s].find(top.getIden());
                    if (it != scopes[s].end()) {
                        found = true;
                        stack.push(ptrValue(it->second));
                        break;
                    }
                }
                if (!found) top.error("identifier is not in scope");*/
                if (top.getBoxLoc() == -1) stack.push(ptrValue(heap.add(top))); // boxing object
                else stack.push(ptrValue(top.getBoxLoc()));
            } else if (OP == OP_DEREFERENCE) {
                // stack order: ptr
                // notation: *ptr
                TOP();
                stack.push(heap.get(top.getPtr()));


            } else if (OP == OP_BEGIN_SCOPE) {
                scopes.push_back(std::map<std::string, size_t>());
            } else if (OP == OP_END_SCOPE) {
                scopes.pop_back();


            } else if (OP == OP_ADD) {
                BASIC_OPERATION(+);
            } else if (OP == OP_SUBTRACT) {
                BASIC_OPERATION(-);
            } else if (OP == OP_MULTIPLY) {
                BASIC_OPERATION(*);
            } else if (OP == OP_MODULO) {
                SIDES();
                stack.push(intValue(lhs.getInt() % rhs.getInt()));
            } else if (OP == OP_DIVIDE) {
                SIDES();
                if (lhs.type == FLOAT) {
                    if (rhs.type == FLOAT) {
                        stack.push(floatValue( lhs.getFloat() / rhs.getFloat() ));
                    } else if (rhs.type == INTIGER) {
                        stack.push(floatValue( lhs.getFloat() / rhs.getInt() ));
                    } else {
                        rhs.error("invalid type for operation"); 
                    }
                } else if (lhs.type == INTIGER) {
                    if (rhs.type == FLOAT) {
                        stack.push(floatValue( lhs.getInt() / rhs.getFloat() ));
                    } else if (rhs.type == INTIGER) {
                        stack.push(floatValue( lhs.getInt() / rhs.getInt() ));
                    } else {
                        rhs.error("invalid type for operation"); 
                    }
                } else {
                    lhs.error("invalid type for operation"); 
                }
            } else if (OP == OP_NEGATE) {
                TOP();
                if (top.type == INTIGER) stack.push(intValue(-top.getInt()));
                else if (top.type == FLOAT) stack.push(floatValue(-top.getFloat()));
                else top.error("invalid type for operation");
            } else if (OP == OP_NOT) {
                TOP();
                stack.push(boolValue(!top.getBool()));
            } else if (OP == OP_AND) {
                SIDES();
                stack.push(boolValue( lhs.getBool() && rhs.getBool() ));
            } else if (OP == OP_OR) {
                SIDES();
                stack.push(boolValue( lhs.getBool() || rhs.getBool() ));


            } else if (OP == OP_PRINT_POP) {
                TOP();
                std::string s = top.toString();
                if (top.type == STRING)
                    std::cout << s.substr(1, s.length()-2);
                else
                    std::cout << s;
            } else if (OP == OP_RETURN_POP) {
                TOP();
                return top;
            }
        }
        #undef OP
        #undef INSTUCTION
        return intValue(0);
    }

};

#endif