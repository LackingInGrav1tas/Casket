#include "opcode.hpp"
#include "memory.hpp"

#include <stack>
#include <iostream>

typedef std::map<std::string, size_t> Scope;

struct Machine {
    std::vector<OpcodeObject> opcode;
    std::vector<Scope> scopes;
    std::stack<Value> stack;
    size_t ip;

    void init(std::vector<OpcodeObject> op) {
        opcode = op;
        ip = 0;
    }

    Value run() {
        #define TOP() Value top = stack.top(); stack.pop()
        #define SIDES() Value rhs = stack.top(); stack.pop(); Value lhs = stack.top(); stack.pop()
        #define INSTRUCTION opcode[ip]
        #define OP INSTRUCTION.op
        for (; ip < opcode.size(); ip++) {
            if (OP == OP_CONSTANT) {
                stack.push(INSTRUCTION.value);

            } else if (OP == OP_SET_VARIABLE) {
                // stack order: new value (ID comes with op)
                // notation: set var = value
                TOP();
                scopes.back()[INSTRUCTION.lexeme] = heap.add(top);
            } else if (OP == OP_EDIT_VARIABLE) {
                // stack order: new value, pointer
                // notation: ptr = new_value
                SIDES();
                heap.change(lhs.getPtr(), rhs);

            } else if (OP == OP_REFERENCE) {
                // stack order: identifier
                // notation: &<id-expr>
                TOP();
                bool found = false;
                for (int s = scopes.size()-1; s >= 0; s--) {
                    auto it = scopes[s].find(top.getIden());
                    if (it != scopes[s].end()) {
                        found = true;
                        stack.push(ptrValue(it->second));
                    }
                }
                if (!found) exit(1); // error
            } else if (OP == OP_DEREFERENCE) {
                // stack order: ptr
                // notation: *ptr
                TOP();
                stack.push(heap.get(top.getPtr()));



            } else if (OP == OP_BEGIN_SCOPE) {
                scopes.push_back(std::map<std::string, size_t>());
            } else if (OP == OP_END_SCOPE) {
                scopes.pop_back();


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