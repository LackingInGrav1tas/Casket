#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "vm.hpp"
#include "memory.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>

Value Machine::run(int argc, char ** argv) {
    // TODO: when a fn returns an object, reset it's box location unless it returns &
    #define POP() if (stack.size() == 0) error("run-time error: stack underflow"); else stack.pop()
    #define TOP() if (stack.size() == 0) error("run-time error: stack underflow"); Value top = stack.top(); POP()
    #define SIDES() Value rhs = stack.top(); POP(); Value lhs = stack.top(); POP()
    #define INSTRUCTION opcode[ip]
    #define OP INSTRUCTION.op

    // template for math operations
    #define BASIC_OPERATION(operator, trait) \
do { \
    SIDES(); \
    if (lhs.type == INSTANCE || rhs.type == INSTANCE) { \
        Value top; \
        Value arg; \
        bool left = false; \
        if (lhs.type == INSTANCE) { \
            top = lhs; \
            arg = rhs; \
            left = true; \
        } else { \
            top = rhs; \
            arg = lhs; \
        } \
        auto it = top.members.find(trait); \
        if (it != top.members.end()) { \
            if (heap.get(it->second).type == FUNCTION) { \
                auto fn = heap.fn_get(heap.get(it->second).getFun()); \
                if (fn.args.size() == 1) { \
                    Scope layer; \
                    layer["this"] = top.box_location; \
                    layer["left"] = heap.add(boolValue(left), scopes.size()); \
                    layer[fn.args[0]] = heap.add(arg, scopes.size()); \
                    fn.vm.scopes.push_back(layer); \
                    fn.vm.templates.push_back(std::map<std::string, ClassTemplate>()); \
                    stack.push(fn.vm.run(argc, argv)); \
                    continue; \
                } else { \
                    std::cout << fn.args.size() << std::endl;\
                    error("run-time error: expected 1 arguement for " + top.toString() + "'s " + trait + " implementation"); \
                } \
            } \
        } \
        lhs.error("invalid type for operation"); \
    } else if (lhs.type == DOUBLE) { \
        if (rhs.type == DOUBLE) { \
            stack.push(doubleValue( lhs.getDouble() operator rhs.getDouble() )); \
        } else if (rhs.type == INTEGER) { \
            stack.push(doubleValue( lhs.getDouble() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else if (lhs.type == INTEGER) { \
        if (rhs.type == DOUBLE) { \
            stack.push(doubleValue( lhs.getInt() operator rhs.getDouble() )); \
        } else if (rhs.type == INTEGER) { \
            stack.push(intValue( lhs.getInt() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else if (lhs.type == POINTER) { \
        stack.push(ptrValue(lhs.getPtr() operator rhs.getInt())); \
    } else { \
        lhs.error("invalid type for operation"); \
    } \
} while (0)

// used as template for boolean operators
#define bBASIC_OPERATION(operator, trait) \
do { \
    SIDES(); \
    if (lhs.type == INSTANCE || rhs.type == INSTANCE) { \
        Value top; \
        Value arg; \
        bool left = false; \
        if (lhs.type == INSTANCE) { \
            top = lhs; \
            arg = rhs; \
            left = true; \
        } else { \
            top = rhs; \
            arg = lhs; \
        } \
        auto it = top.members.find(trait); \
        if (it != top.members.end()) { \
            if (heap.get(it->second).type == FUNCTION) { \
                auto fn = heap.fn_get(heap.get(it->second).getFun()); \
                if (fn.args.size() == 1) { \
                    Scope layer; \
                    layer["this"] = top.box_location; \
                    layer["left"] = heap.add(boolValue(left), scopes.size()); \
                    layer[fn.args[0]] = heap.add(arg, scopes.size()); \
                    fn.vm.scopes.push_back(layer); \
                    fn.vm.templates.push_back(std::map<std::string, ClassTemplate>()); \
                    stack.push(fn.vm.run(argc, argv)); \
                    continue; \
                } else { \
                    error("run-time error: expected 1 arguement for " + top.toString() + "'s " + trait + " implementation"); \
                } \
            } \
        } \
        lhs.error("invalid type for operation"); \
    } else if (lhs.type == DOUBLE) { \
        if (rhs.type == DOUBLE) { \
            stack.push(boolValue( lhs.getDouble() operator rhs.getDouble() )); \
        } else if (rhs.type == INTEGER) { \
            stack.push(boolValue( lhs.getDouble() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else if (lhs.type == INTEGER) { \
        if (rhs.type == DOUBLE) { \
            stack.push(boolValue( lhs.getInt() operator rhs.getDouble() )); \
        } else if (rhs.type == INTEGER) { \
            stack.push(boolValue( lhs.getInt() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else { \
        lhs.error("invalid type for operation"); \
    } \
} while (0)

    for (; ip < opcode.size(); ip++) {
        // printOp(opcode[ip]);
        // std::cout << std::endl;
        // std::cout << "#: " << ip << "  type: " << OP << std::endl;

// <VARIABLES>
        if (OP == OP_CONSTANT) {
            // pushes literal value
            stack.push(INSTRUCTION.value);

        } else if (OP == OP_SET_VARIABLE) {
            // stack order: new value (ID comes with op)
            // notation: set var = value
            TOP();
            scopes.back()[INSTRUCTION.lexeme] = heap.add(top, scopes.size());
        } else if (OP == OP_GET_VARIABLE) {
            // retrieving value from variable
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
            if (!found) {
                size_t distance = -1;
                std::string match = "";

                for (int i = 0; i < scopes.size(); i++) {
                    for (auto it = scopes[i].begin(); it != scopes[i].end(); it++) {
                        if (distance == -1) {
                            match = it->first;
                            distance = LevenshteinDistance(match, top.getIden());
                        } else {
                            if (distance > LevenshteinDistance(it->first, top.getIden())) {
                                match = it->first;
                                distance = LevenshteinDistance(match, top.getIden());
                            }
                        }
                    }
                }

                top.error("identifier is not in scope, did you mean \"" + match + "\"?");
            }
        } else if (OP == OP_EDIT_VARIABLE) {
            // stack order: new value, pointer
            // notation: ptr = new_value
            SIDES();
            bool found = false;
            int scope = scopes.size()-1;
            for (; scope >= 0; scope--) {
                for (auto it = scopes[scope].begin(); it != scopes[scope].end(); it++) {
                    if (it->second == lhs.getBoxLoc()) {
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) error("run-time error: cannot edit a non-existent variable");
            // PRINT("CHANGE S: " << scope);
            heap.change(lhs.getBoxLoc(), rhs.edit(lhs.getBoxLoc()), scope);
        } else if (OP == OP_REFERENCE) {
            // stack order: identifier
            // notation: &<id-expr>
            TOP();
            if (top.box_location == -1) stack.push(ptrValue(heap.add(top, scopes.size()))); // boxing object
            else stack.push(ptrValue(top.getBoxLoc()));
        } else if (OP == OP_DEREFERENCE) {
            // stack order: ptr
            // notation: *ptr
            TOP();
            stack.push(heap.get(top.getPtr()));
        } else if (OP == OP_COPY) {
            // notation: ptr = new_value
            // sets box location to null, basically making a new copy of a value in memory
            TOP();
            top.box_location = -1;
            stack.push(top);

// <OPERATIONS>
        } else if (OP == OP_ADD) {
            BASIC_OPERATION(+, "+");
        } else if (OP == OP_SUBTRACT) {
            BASIC_OPERATION(-, "-");
            
        } else if (OP == OP_MULTIPLY) {
            BASIC_OPERATION(*, "*");
            
        } else if (OP == OP_MODULO) {
            // modulo + string concatenation
            SIDES();
            if (lhs.type == INSTANCE || rhs.type == INSTANCE) {
                Value top;
                Value arg;
                bool left = false;
                if (lhs.type == INSTANCE) {
                    top = lhs;
                    arg = rhs;
                    left = true;
                } else {
                    top = rhs;
                    arg = lhs;
                }
                auto it = top.members.find("%");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            layer["left"] = heap.add(boolValue(left), scopes.size());
                            layer[fn.args[0]] = heap.add(arg, scopes.size());
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 1 arguement for " + top.toString() + "'s % implementation");
                        }
                    }
                } else if (lhs.type == STRING || rhs.type == STRING) {
                    stack.push(strValue(trim(lhs) + trim(rhs)));
                } else {
                    stack.push(intValue(lhs.getInt() % rhs.getInt()));
                }
            } else if (lhs.type == STRING || rhs.type == STRING) {
                stack.push(strValue(trim(lhs) + trim(rhs)));
            } else {
                stack.push(intValue(lhs.getInt() % rhs.getInt()));
            }
        } else if (OP == OP_DIVIDE) {
            SIDES();
            if (lhs.type == DOUBLE && ( rhs.type == DOUBLE || rhs.type == INTEGER )) {
                if (rhs.type == DOUBLE) {
                    if (rhs.getDouble() == 0) rhs.error("division by zero");
                    stack.push(doubleValue( lhs.getDouble() / rhs.getDouble() ));
                } else if (rhs.type == INTEGER) {
                    if (rhs.getInt() == 0) rhs.error("division by zero");
                    stack.push(doubleValue( lhs.getDouble() / rhs.getInt() ));
                } else {
                    rhs.error("invalid type for operation"); 
                }
            } else if (lhs.type == INTEGER && ( rhs.type == DOUBLE || rhs.type == INTEGER )) {
                if (rhs.type == DOUBLE) {
                    if (rhs.getDouble() == 0) rhs.error("division by zero");
                    stack.push(doubleValue( lhs.getInt() / rhs.getDouble() ));
                } else if (rhs.type == INTEGER) {
                    if (rhs.getInt() == 0) rhs.error("division by zero");
                    stack.push(intValue( lhs.getInt() / rhs.getInt() ));
                } else {
                    rhs.error("invalid type for operation"); 
                }
            } else {
                if (lhs.type == INSTANCE || rhs.type == INSTANCE) {
                    Value top;
                    Value arg;
                    if (lhs.type == INSTANCE) {
                        top = lhs;
                        arg = rhs;
                    } else {
                        top = rhs;
                        arg = lhs;
                    }
                    auto it = top.members.find("/");
                    if (it != top.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 1) {
                                Scope layer;
                                layer["this"] = top.box_location;
                                layer[fn.args[0]] = heap.add(arg, scopes.size());
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                stack.push(fn.vm.run(argc, argv));
                            } else {
                                error("run-time error: expected 1 arguement for " + top.toString() + "'s / implementation");
                            }
                        }
                    } else lhs.error("expected either an instance or a number.");
                } else {
                    lhs.error("expected either an instance or a number.");
                }
            }

        } else if (OP == OP_MORE) {
            bBASIC_OPERATION(>, ">");
        } else if (OP == OP_MORE_EQ) {
            bBASIC_OPERATION(>=, ">=");
        } else if (OP == OP_LESS) {
            bBASIC_OPERATION(<, "<");
        } else if (OP == OP_LESS_EQ) {
            bBASIC_OPERATION(<=, "<=");

        } else if (OP == OP_EQUALITY) {
            SIDES();
            if (lhs.type == INSTANCE || rhs.type == INSTANCE) {
                Value top;
                Value arg;
                bool left = false;
                if (lhs.type == INSTANCE) {
                    top = lhs;
                    arg = rhs;
                    left = true;
                } else {
                    top = rhs;
                    arg = lhs;
                }
                auto it = top.members.find("==");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            layer["left"] = heap.add(boolValue(left), scopes.size());
                            layer[fn.args[0]] = heap.add(arg, scopes.size());
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 1 arguement for " + top.toString() + "'s == implementation");
                        }
                    }
                } else lhs.error("invalid type for operation");
            } else {
                switch (lhs.type) {
                    case STRING:
                        if (rhs.type != STRING) stack.push(boolValue(false));
                        else stack.push(boolValue(lhs.getStr() == rhs.getStr()));
                        break;
                    case INTEGER:
                        if (rhs.type != INTEGER) stack.push(boolValue(false));
                        else stack.push(boolValue(lhs.getInt() == rhs.getInt()));
                        break;
                    case DOUBLE:
                        if (rhs.type != DOUBLE) stack.push(boolValue(false));
                        else stack.push(boolValue(lhs.getDouble() == rhs.getInt()));
                        break;
                    case BOOLEAN:
                        if (rhs.type != BOOLEAN) stack.push(boolValue(false));
                        else stack.push(boolValue(lhs.getBool() == rhs.getBool()));
                        break;
                    case NIL:
                        stack.push(boolValue( rhs.type == NIL ));
                        break;                    
                    default:
                        stack.push(boolValue(lhs.toString() == rhs.toString()));
                        break;
                }
            }
        } else if (OP == OP_NOT_EQUAL) {
            SIDES();
            if (lhs.type == INSTANCE || rhs.type == INSTANCE) {
                Value top;
                Value arg;
                bool left = false;
                if (lhs.type == INSTANCE) {
                    top = lhs;
                    arg = rhs;
                    left = true;
                } else {
                    top = rhs;
                    arg = lhs;
                }
                auto it = top.members.find("%");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            layer["left"] = heap.add(boolValue(left), scopes.size());
                            layer[fn.args[0]] = heap.add(arg, scopes.size());
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 1 arguement for " + top.toString() + "'s % implementation");
                        }
                    }
                } else lhs.error("invalid type for operation");
            } else {
                switch (lhs.type) {
                    case STRING:
                        if (rhs.type != STRING) stack.push(boolValue(true));
                        else stack.push(boolValue(lhs.getStr() != rhs.getStr()));
                        break;
                    case INTEGER:
                        if (rhs.type != INTEGER) stack.push(boolValue(true));
                        else stack.push(boolValue(lhs.getInt() != rhs.getInt()));
                        break;
                    case DOUBLE:
                        if (rhs.type != DOUBLE) stack.push(boolValue(true));
                        else stack.push(boolValue(lhs.getDouble() != rhs.getInt()));
                        break;
                    case BOOLEAN:
                        if (rhs.type != BOOLEAN) stack.push(boolValue(true));
                        else stack.push(boolValue(lhs.getBool() != rhs.getBool()));
                        break;
                    case NIL:
                        stack.push(boolValue( rhs.type != NIL ));
                        break;                    
                    default:
                        stack.push(boolValue(lhs.toString() != rhs.toString()));
                        break;
                }
            }

        } else if (OP == OP_NEGATE) {
            TOP();
            if (top.type == INSTANCE) {
                auto it = top.members.find("prefix-");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 0) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 0 arguements for " + top.toString() + "'s prefix- implementation");
                        }
                    }
                } else top.error("invalid type for operation");
            }
            else if (top.type == INTEGER) stack.push(intValue(-top.getInt()));
            else if (top.type == DOUBLE) stack.push(doubleValue(-top.getDouble()));
            else top.error("invalid type for operationn");
        } else if (OP == OP_NOT) {
            TOP();
            if (top.type == INSTANCE) {
                auto it = top.members.find("!");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 0) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 0 arguements for " + top.toString() + "'s ! implementation");
                        }
                    }
                } else top.error("invalid type for operation!");
            } else
                stack.push(boolValue(!top.getBool()));
        } else if (OP == OP_AND) {
            SIDES();
            stack.push(boolValue( lhs.getBool() && rhs.getBool() ));
        } else if (OP == OP_OR) {
            SIDES();
            stack.push(boolValue( lhs.getBool() || rhs.getBool() ));
        } else if (OP == OP_BIT_AND) {
            SIDES();
            stack.push(byteValue( lhs.getByte() & rhs.getByte() ));
        } else if (OP == OP_BIT_OR) {
            SIDES();
            stack.push(byteValue( lhs.getByte() | rhs.getByte() ));
        } else if (OP == OP_XOR) {
            SIDES();
            if (lhs.type == BOOLEAN) {
                stack.push(boolValue( lhs.getBool() ^ rhs.getBool() ));
            } else {
                stack.push(byteValue( lhs.getByte() ^ rhs.getByte() ));
            }
        } else if (OP == OP_INCREMENT) {
            // increments, ++
            TOP();
            switch (top.type) {
                case INTEGER: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            intValue(
                                heap.get(
                                    top.box_location
                                ).getInt()+1
                            ),
                            scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(intValue(top.getInt() + 1));
                    }
                    break;
                }
                case DOUBLE: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            doubleValue(
                                heap.get(
                                    top.box_location
                                ).getDouble()+1
                            ), scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(doubleValue(top.getDouble() + 1));
                    }
                    break;
                }
                case POINTER: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            ptrValue(
                                heap.get(
                                    top.box_location
                                ).getPtr()+1
                            ), scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(ptrValue(top.getPtr() + 1));
                    }
                    break;
                }
                case INSTANCE: {
                    auto it = top.members.find("++");
                    if (it != top.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 0) {
                                Scope layer;
                                layer["this"] = top.box_location;
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                stack.push(fn.vm.run(argc, argv));
                                break;
                            } else {
                                error("run-time error: expected 0 arguements for " + top.toString() + "'s ++ implementation");
                            }
                        }
                    }
                }
                default: {
                    error("run-time error: wrong type for increment: " + top.toString());
                }
            }
        } else if (OP == OP_DECREMENT) {
            // decrements, --
            TOP();
            switch (top.type) {
                case INTEGER: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            intValue(
                                heap.get(
                                    top.box_location
                                ).getInt()-1
                            ), scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(intValue(top.getInt() - 1));
                    }
                    break;
                }
                case DOUBLE: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            doubleValue(
                                heap.get(
                                    top.box_location
                                ).getDouble()-1
                            ), scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(doubleValue(top.getDouble() - 1));
                    }
                    break;
                }
                case POINTER: {
                    if (top.box_location != -1) {
                        heap.change(
                            top.box_location,
                            ptrValue(
                                heap.get(
                                    top.box_location
                                ).getPtr()-1
                            ), scopes.size()
                        );
                        stack.push(heap.get(top.box_location));
                    } else {
                        stack.push(ptrValue(top.getPtr() - 1));
                    }
                    break;
                }
                case INSTANCE: {
                    auto it = top.members.find("--");
                    if (it != top.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 0) {
                                Scope layer;
                                layer["this"] = top.box_location;
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                stack.push(fn.vm.run(argc, argv));
                                break;
                            } else {
                                error("run-time error: expected 0 arguements for " + top.toString() + "'s -- implementation");
                            }
                        }
                    }
                }
                default: {
                    error("run-time error: wrong type for decrement: " + top.toString());
                }
            }

// <CLASSES>
        } else if (OP == OP_DECL_CLASS) {
            // creates a class
            ClassTemplate t;
            for (int i = 0; i < opcode[ip].i; i++) {
                auto value = stack.top();
                stack.pop();
                std::string id = stack.top().getIden();
                stack.pop();
                // std::cout << "\nmember: " << id << "\nvalue: " << value.toString() << std::endl;
                t.members[id] = value;
            }
            templates.back()[opcode[ip].lexeme] = t;
        }

        else if (OP == OP_CREATE_INST) {
            // creates an instance of a class, new Class(a = b...)
            ClassTemplate templt;
            std::string classname;
            bool found = false;
            for (int s = templates.size()-1; s >= 0; s--) {
                auto it = templates[s].find(opcode[ip].lexeme);
                if (it != templates[s].end()) {
                    found = true;
                    templt = it->second;
                    classname = it->first;
                    break;
                }
            }
            if (!found) error("run-time error: couldn't find class " + opcode[ip].lexeme + " in scope.");

            for (int i = 0; i < opcode[ip].i; i++) {
                Value value = stack.top();
                stack.pop();
                std::string member = stack.top().getIden();
                stack.pop();
                if (templt.members.find(member) != templt.members.end()) {
                    // std::cout << (value.type == STRING ? "is str: " + member : "not str: " + member) << std::endl;
                    templt.members[member] = value;
                } else {
                    error("run-time error: member " + member + " does not exist.");
                }
            }

<<<<<<< Updated upstream
            stack.push(instanceValue(templt, scopes.size()));
=======
            stack.push(instanceValue(templt, classname));
>>>>>>> Stashed changes

        } else if (OP == OP_GET_MEMBER) {
            // get field
            SIDES();
            if (lhs.members.find(rhs.getIden()) != lhs.members.end())
                stack.push(heap.get(lhs.members[rhs.getIden()]));
            else do {
                // allows primitive types member functions
                stack.push(lhs);
                switch (lhs.type) {
                    case INTEGER: {

                    }
                    case DOUBLE: {

                    }
                    case STRING: {
                        if (rhs.getIden() == "length") {
                            stack.push(stlValue(STRING_LENGTH));
                            continue;
                        } else if (rhs.getIden() == "substring") {
                            stack.push(stlValue(STRING_SUBSTRING));
                            continue;
                        } else {
                            error("the only member functions string supports are: length, substring; not " + rhs.getIden());
                        }

                    }
                    case BOOLEAN: {

                    }
                    case BYTE: {
                        if (rhs.getIden() == "get_bit") {
                            stack.push(stlValue(BYTE_GET_BIT));
                            continue;
                        } else if (rhs.getIden() == "set_bit") {
                            stack.push(stlValue(BYTE_SET_BIT));
                            continue;
                        } else {
                            error("the only member functions byte supports are: get_bit, set_bit; not " + rhs.getIden());
                        }
                    }
                    case LIST: {
                        if (rhs.getIden() == "to_string") {
                            stack.push(stlValue(LIST_TO_STRING));
                            continue;
                        } else if (rhs.getIden() == "join") {
                            stack.push(stlValue(LIST_JOIN));
                            continue;
                        } else if (rhs.getIden() == "insert") {
                            stack.push(stlValue(LIST_INSERT));
                            continue;
                        } else if (rhs.getIden() == "remove") {
                            stack.push(stlValue(LIST_REMOVE));
                            continue;
                        } else if (rhs.getIden() == "size") {
                            stack.push(stlValue(LIST_SIZE));
                            continue;
                        } else if (rhs.getIden() == "push") {
                            stack.push(stlValue(LIST_PUSH));
                            continue;
                        } else if (rhs.getIden() == "pop") {
                            stack.push(stlValue(LIST_POP));
                            continue;
                        } else {
                            error("the only member functions list supports are: to_string, join, insert, remove, size, push, pop; not " + rhs.getIden());
                        }
                    }
                }
                std::string message = "run-time error: object {" + lhs.toString() + "} has no member " + rhs.getIden() + ". did you mean: [";
                for (auto it = lhs.members.begin(); it != lhs.members.end(); it++) {
                    message += it->first + ", ";
                }
                if (message.back() == ' ') {
                    message.pop_back();
                    message.pop_back();
                }
                error(message + "]");
            } while (0);

// <SCOPE>
        } else if (OP == OP_BEGIN_SCOPE) {
            // adds scope to scopes
            scopes.push_back(std::map<std::string, size_t>());
            templates.push_back(std::map<std::string, ClassTemplate>());

        } else if (OP == OP_END_SCOPE) {
            // removes scope + clears memory
            if (scopes.size() == 0) error("run-time error: scope underflow - end_scope");
            auto s = scopes.back();

            for (auto it = s.begin(); it != s.end(); it++) { // garbage collection
                heap.dump(it->second, scopes.size());
            }

            scopes.pop_back();
            templates.pop_back();

// <FUNCTIONS>
        } else if (OP == OP_CALL_FN) {
            // calls function / stl function
            std::vector<Value> args;
            for (int i = 0; i < INSTRUCTION.i; i++) {
                TOP();
                args.insert(args.begin(), top);
            }
            TOP();
            if (top.type == STL_CALL) {
                switch (top.getSTL()) { // prim stlcall [args] OP_CALL
                    case STRING_LENGTH: {
                        if (INSTRUCTION.i != 0) {
                            error("run-time error: string.length expects 0 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        stack.push( intValue( prim.getStr().length() ) );
                        break;
                    }
                    case STRING_SUBSTRING: {
                        if (INSTRUCTION.i != 2) {
                            error("run-time error: string.substring expects 2 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        stack.push( strValue( prim.getStr().substr(args[0].getInt(), args[1].getInt()) ) );
                        break;
                    }
                    case LIST_TO_STRING: {
                        if (INSTRUCTION.i != 0) {
                            error("run-time error: list.to_string expects 0 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        stack.push( strValue( prim.toString() ) );
                        break;
                    }
                    case LIST_SIZE: {
                        if (INSTRUCTION.i != 0) {
                            error("run-time error: list.size expects 0 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        stack.push( intValue( prim.getList().size() ) );
                        break;
                    }
                    case LIST_JOIN: {
                        if (INSTRUCTION.i != 1) {
                            error("run-time error: list.join expects 1 arguement");
                        }
                        std::string connector = trim(args[0]);
                        Value prim = stack.top();
                        stack.pop();
                        if (prim.getList().size() == 0) {
                            stack.push(strValue(""));
                        } else {
                            std::string s = "";
                            for (int i = 0; i < prim.getList().size() - 1; i++) {
                                s += heap.get(prim.getList()[i]).toString() + connector;
                            }
                            // std::cout << "pushing" << std::endl;
                            stack.push(
                                strValue(
                                    s + heap.get(prim.getList()[prim.getList().size() - 1]).toString()
                                )
                            );
                        }
                        break;
                    }
                    case LIST_INSERT: {
                        if (INSTRUCTION.i != 2) {
                            error("run-time error: list.insert expects 2 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        prim.getList().insert( prim.getList().begin() + args[0].getInt(), heap.add( args[1], scopes.size() ) );
                        heap.change(prim.getBoxLoc(), prim, scopes.size());
                        break;
                    }
                    case LIST_REMOVE: {
                        if (INSTRUCTION.i != 1) {
                            error("run-time error: list.remove expects 1 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        prim.getList().erase(prim.getList().begin() + args[0].getInt());
                        heap.change(prim.getBoxLoc(), prim, scopes.size());
                        break;
                    }
                    case LIST_PUSH: {
                        if (INSTRUCTION.i != 1) {
                            error("run-time error: list.push expects 1 arguement");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        prim.getList().push_back(
                            heap.add( args[0], scopes.size() )
                        );
                        heap.change(prim.getBoxLoc(), prim, scopes.size());
                        break;
                    }
                    case LIST_POP: {
                        if (INSTRUCTION.i != 0) {
                            error("run-time error: list.pop expects 0 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        int last = prim.list_locations.back();
                        prim.list_locations.pop_back();
                        heap.change(prim.getBoxLoc(), prim, scopes.size());
                        stack.push(
                            heap.get(last)
                        );
                        break;
                    }
                    case BYTE_GET_BIT: {
                        if (INSTRUCTION.i != 1) {
                            error("run-time error: byte.get_bit expects 1 arguement");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        stack.push( boolValue(((prim.getByte() >> args[0].getInt())  & 0x01)) );
                        break;
                    }
                    case BYTE_SET_BIT: {
                        if (INSTRUCTION.i != 2) {
                            error("run-time error: byte.set_bit expects 2 arguements");
                        }
                        Value prim = stack.top();
                        stack.pop();
                        prim.getByte() ^= (-args[1].getBool() ^ prim.getByte()) & (1UL << args[0].getInt());
                        heap.change(prim.getBoxLoc(), prim, scopes.size());
                        break;
                    }
                    
                    default: {
                        error("run-time error: stl call does not exist.");
                    }
                }
            } else {
                int fn_loc = top.getFun();
                Function f = heap.fn_get(fn_loc);
                if (f.args.size() != args.size()) error("run-time error: expected " + std::to_string(f.args.size()) + " arguments, found " + std::to_string(args.size()));
                f.vm.scopes = scopes;
                f.vm.scopes.push_back(Scope());
                f.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                for (int i = 0; i < f.args.size(); i++) {
                    f.vm.scopes.back()[f.args[i]] = heap.add(args[i], scopes.size());
                }
                if (top.home_location != -1) {
                    f.vm.scopes.back()["this"] = top.home_location;
                }
                Value return_value = f.vm.run(argc, argv);
                return_value.box_location = -1;
                stack.push(return_value);
                for (int i = 0; i < scopes.size(); i++) scopes[i] = f.vm.scopes[i];
            }
// <MISC>
        } else if (OP == OP_RETURN_POP) {
            // returns
            TOP();
            return top;
        }

        // control flow
        else if (OP == OP_JUMP) {
            ip += INSTRUCTION.i;
        } else if (OP == OP_JUMP_LOOP) {
            ip += INSTRUCTION.i;
        } else if (OP == OP_JUMP_FALSE) {
            TOP();
            if (!top.getBool()) {
                ip += INSTRUCTION.i;
            }
        }

        else if (OP == OP_BREAK) {
            int position = ip;
            for (; OP != OP_JUMP_LOOP; ip++)
                if (ip >= opcode.size()) {
                    error("run-time error: misplaced break");
                }
            ip++;
        } else if (OP == OP_CONTINUE) {
            int position = ip;
            for (; OP != OP_JUMP_LOOP; ip++)
                if (ip >= opcode.size()) {
                    error("run-time error: misplaced continue");
                }
            ip--;
        }

        else if (OP == OP_GOTO_LABEL) {
            // goto in language
            for (int i = 0; ; i++) {
                if (i >= opcode.size()) {
                    error("run-time error: couldn't find the label" + INSTRUCTION.lexeme);
                }
                if (opcode[i].op == OP_LABEL) {
                    if (opcode[i].lexeme == INSTRUCTION.lexeme) {
                        ip = i;
                        break;
                    }
                }
            }
        
        } else if (OP == OP_CREATE_LIST) {
            // creates a list, allows for literal
            Locations list;
            std::vector<Value> values;
            for (int i = 0; i < opcode[ip].i; i++) {
                TOP();
                values.push_back(top);
            }
            std::reverse(values.begin(), values.end());
            for (int i = 0; i < values.size(); i++) {
                list.push_back(heap.add(values[i], scopes.size()));
            }
            stack.push(listValue(list));

// <STANDARD LIBRARY>
        
        } else if (OP == OP_INPUT) {
            // standard library: prints, returns input
            std::string buffer;
            std::getline(std::cin, buffer);
            stack.push(strValue(buffer));    

        } else if (OP == OP_DEBUG_SCOPES) {
            // standard library: prints out scopes
            for (int i = 0; i < scopes.size(); i++) {
                std::cout << "\n ==SCOPE " << i << "== " << std::endl;
                for (auto p = scopes[i].begin(); p != scopes[i].end(); p++) {
                    std::cout << "[@" << p->second << "] " << p->first << " : " << heap.get(p->second).toString() << std::endl;
                }
            }
        } else if (OP == OP_TYPES_IS) {
            // standard library: returns true if value is type
            Value type = stack.top();
            stack.pop();
            Value val = stack.top();
            stack.pop();

            stack.push(boolValue(val.type == type.getInt()));
        
        } else if (OP == OP_TYPES_TO) {
            // standard library: returns true if value is type
            Value type = stack.top();
            stack.pop();
            Value val = stack.top();
            stack.pop();

            switch (val.type) {
                case INTEGER: {
                    switch (type.getInt()) {
                        case BYTE: {
                            stack.push(byteValue(val.getInt()));
                            break;
                        }
                        case DOUBLE: {
                            stack.push(doubleValue(val.getInt()));
                            break;
                        }
                        case STRING: {
                            stack.push(strValue(std::to_string(val.getInt())));
                            break;
                        }
                        default: error("run-time error: byte objects cannot be converted to anything other than int, double, string");
                    }
                    break;
                }
                case DOUBLE: {
                    switch (type.getInt()) {
                        case INTEGER: {
                            stack.push(intValue(val.getDouble()));
                            break;
                        }
                        case STRING: {
                            stack.push(strValue(std::to_string(val.getDouble())));
                            break;
                        }
                        case BYTE: {
                            stack.push(byteValue(val.getDouble()));
                            break;
                        }
                        default: error("run-time error: double objects cannot be converted to anything other than int, string, byte");
                    }
                    break;
                }
                case STRING: {
                    switch (type.getInt()) {
                        case INTEGER: {
                            stack.push(intValue(std::stoi(val.getStr())));
                            break;
                        }
                        case DOUBLE: {
                            stack.push(doubleValue(std::stod(val.getStr())));
                            break;
                        }
                        default: error("run-time error: string objects cannot be converted to anything other than int, double");
                    }
                    break;
                }
                case BOOLEAN: {
                    switch (type.getInt()) {
                        case INTEGER: {
                            stack.push(intValue(val.getBool()));
                            break;
                        }
                        case DOUBLE: {
                            stack.push(doubleValue(val.getBool()));
                            break;
                        }
                        default: error("run-time error: bool objects cannot be converted to anything other than int, double");
                    }
                    break;
                }
                case NIL: {
                    error("run-time error: null cannot be converted");
                }
                case BYTE: {
                    switch (type.getInt()) {
                        case INTEGER: {
                            stack.push(intValue(val.getByte()));
                            break;
                        }
                        case DOUBLE: {
                            stack.push(doubleValue(val.getByte()));
                            break;
                        }
                        case STRING: {
                            stack.push(strValue(std::string(1, (char)val.getByte())));
                            break;
                        }
                        default: error("run-time error: byte objects cannot be converted to anything other than int, double, string");
                    }
                    break;
                }
                case POINTER: {
                    if (type.getInt() == INTEGER) {
                        stack.push(intValue(val.getPtr()));
                    } else error("run-time error: pointer objects cannot be converted to anything other than int");
                    break;
                }
                case LIST: {
                    error("run-time error: list objects cannot be converted");
                }
                case FUNCTION: {
                    error("run-time error: function objects cannot be converted");
                }
                case INSTANCE: {
                    error("run-time error: instance objects cannot be converted");
                }
                default: error("run-time error: type does not exist.");
            }

        } else if (OP == OP_TYPES_GET_TYPE) {
            // standard library: returns value type
            TOP();
            stack.push(intValue(top.type));

        } else if (OP == OP_ENVIRON_ARGS) {
            // standard library: returns args
            Value args = listValue({});
            for (int i = 0; i < argc; i++) {
                args.list_locations.push_back(heap.add(strValue((std::string)argv[i]), scopes.size()));
            }
            stack.push(args);

        } else if (OP == OP_STREAM_FILE_READ_BYTE) {
            // standard library: reads file as list of bytes
            TOP();
            std::ifstream file(trim(top));
            std::stringstream file_buffer;
            file_buffer << file.rdbuf();
            Locations locs;
            for (int i = 0; i < file_buffer.str().length(); i++) {
                locs.push_back(
                    heap.add(
                        byteValue(file_buffer.str()[i]), scopes.size()
                    )
                );
            }
            stack.push(listValue(locs));


        } else if (OP == OP_STREAM_FILE_WRITE_BYTE) {
            // standard library: writes list to file
            Value message = stack.top();
            stack.pop();
            Value file = stack.top();
            stack.pop();

            std::ofstream ofile(trim(file));
            Locations list = message.getList();
            for (int i = 0; i < list.size(); i++) {
                ofile << trim(heap.get(list[i]));
            }
            ofile.close();

        } else if (OP == OP_ENVIRON_COMMAND) {
            // standard library: runs command + returns output
            TOP();
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(top.toString().c_str(), "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("Environment.command failed");
            }
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                result += buffer.data();
            }
            stack.push(strValue(result));

        } else if (OP == OP_ENVIRON_EXIT) {
            // standard library: exits program
            TOP();
            return top;
        
        } else if (OP == OP_ENVIRON_CASKET) {
            // standard library: calls native code
            TOP();
            Lexer lexer(trim(top));
            Machine vm;
            vm.init(lexer);
            vm.initialize_standard_library();
            stack.push(vm.run(argc, argv));

        } else if (OP == OP_STREAM_FILE_READ) {
            // standard library: read file as string
            TOP();
            std::ifstream file(trim(top));
            std::stringstream file_buffer;
            file_buffer << file.rdbuf();
            stack.push(strValue(file_buffer.str()));

        } else if (OP == OP_STREAM_FILE_WRITE) {
            // standard library: write string to file
            Value message = stack.top();
            stack.pop();
            Value file = stack.top();
            stack.pop();

            std::ofstream ofile(trim(file));
            ofile << trim(message);
            ofile.close();

        } else if (OP == OP_PRINT_POP) {
            // standard library: prints to stdout
            TOP();
            std::cout << trim(top);

        } else if (OP == OP_INDEX) {
            // standard library: indexes object, xyz[]
            SIDES();
            if (lhs.type == STRING) {
                if (lhs.getStr().length() <= rhs.getInt()) error("run-time error: index " + std::to_string(rhs.getInt()) + " out of range.");
                else stack.push(strValue(std::string(1, lhs.getStr().at(rhs.getInt()))));
            } else if (lhs.type == LIST) {
                stack.push(heap.get(lhs.list_locations[rhs.getInt()]));
            } else if (lhs.type == INSTANCE) {
                auto it = lhs.members.find("[");
                if (it != lhs.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = lhs.box_location;
                            layer[fn.args[0]] = heap.add(rhs, scopes.size());
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run(argc, argv));
                        } else {
                            error("run-time error: expected 1 arguement for " + lhs.toString() + "'s [ implementation");
                        }
                    }
                } else {
                    error("run-time error: cannot index a non-string, non-list object : " + lhs.toString());
                }
            } else {
                error("run-time error: cannot index a non-string, non-list object : " + lhs.toString());
            }
        }
    }
    #undef OP
    #undef INSTUCTION
    return intValue(0);
}

#endif