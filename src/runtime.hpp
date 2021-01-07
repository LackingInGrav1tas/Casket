#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "vm.hpp"
#include "memory.hpp"

Value Machine::run() {
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
            heap.change(lhs.getBoxLoc(), rhs.edit(lhs.getBoxLoc()));

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
            if (top.box_location == -1) stack.push(ptrValue(heap.add(top))); // boxing object
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

#endif