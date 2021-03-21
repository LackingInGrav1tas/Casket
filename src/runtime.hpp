#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "vm.hpp"
#include "memory.hpp"

Value Machine::run() {
    // TODO: when a fn returns an object, reset it's box location unless it returns &
    #define POP() if (stack.size() == 0) error("run-time error: stack underflow"); else stack.pop()
    #define TOP() if (stack.size() == 0) error("run-time error: stack underflow"); Value top = stack.top(); POP()
    #define SIDES() Value rhs = stack.top(); POP(); Value lhs = stack.top(); POP()
    #define INSTRUCTION opcode[ip]
    #define OP INSTRUCTION.op

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
                    layer["left"] = heap.add(boolValue(left)); \
                    layer[fn.args[0]] = heap.add(arg); \
                    fn.vm.scopes.push_back(layer); \
                    fn.vm.templates.push_back(std::map<std::string, ClassTemplate>()); \
                    stack.push(fn.vm.run()); \
                    continue; \
                } \
            } \
        } \
        lhs.error("invalid type for operation"); \
    } else if (lhs.type == FLOAT) { \
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
#define bBASIC_OPERATION(operator) \
do { \
    SIDES(); \
    if (lhs.type == FLOAT) { \
        if (rhs.type == FLOAT) { \
            stack.push(boolValue( lhs.getFloat() operator rhs.getFloat() )); \
        } else if (rhs.type == INTIGER) { \
            stack.push(boolValue( lhs.getFloat() operator rhs.getInt() )); \
        } else { \
            rhs.error("invalid type for operation"); \
        } \
    } else if (lhs.type == INTIGER) { \
        if (rhs.type == FLOAT) { \
            stack.push(boolValue( lhs.getInt() operator rhs.getFloat() )); \
        } else if (rhs.type == INTIGER) { \
            stack.push(boolValue( lhs.getInt() operator rhs.getInt() )); \
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
            templates.push_back(std::map<std::string, ClassTemplate>());
        } else if (OP == OP_END_SCOPE) {
            if (scopes.size() == 0) error("run-time error: scope underflow - end_scope");
            auto s = scopes.back();

            for (auto it = s.begin(); it != s.end(); it++) { // garbage collection
                heap.dump(it->second);
            }

            scopes.pop_back();
            templates.pop_back();


        } else if (OP == OP_ADD) {
            BASIC_OPERATION(+, "operator_add");
            /*if (stack.top().type == INTIGER || stack.top().type == FLOAT) {
                BASIC_OPERATION(+);
            } else if (stack.top().type == INSTANCE) {
                TOP();
                auto it = top.members.find("operator_add");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            layer[fn.args[0]] = heap.add(stack.top());
                            stack.pop();
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run());
                            continue;
                        }
                    }
                }
            } else if (stack.top().type == STRING) {
                SIDES();
                stack.push(strValue(lhs.getStr() + rhs.getStr()));
            } else {
                stack.top().error("expected either an instance, string, or number.");
            }*/
        } else if (OP == OP_SUBTRACT) {
            BASIC_OPERATION(-, "operator_subtract");
            
        } else if (OP == OP_MULTIPLY) {
            BASIC_OPERATION(*, "operator_multiply");
            
        } else if (OP == OP_MODULO) {
            SIDES();
            if (lhs.type == STRING) {
                std::string converted;
                if (rhs.type == INSTANCE) {
                    auto it = rhs.members.find("to_string");
                    if (it != rhs.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 0) {
                                Scope layer;
                                layer["this"] = rhs.box_location;
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                converted = fn.vm.run().getStr();
                            } else {
                                error("run-time error: invalid type for operation  object: " + rhs.toString());
                            }
                        } else {
                            error("run-time error: invalid type for operation  object: " + rhs.toString());
                        }
                    } else {
                        error("run-time error: invalid type for operation  object: " + rhs.toString());
                    }
                } else {
                    converted = rhs.toString();
                    if (converted.at(0) == '"') {
                        converted = rhs.str;
                    }
                }
                stack.push(strValue(lhs.getStr() + converted));
            } else if (rhs.type == STRING) {
                std::string converted;
                if (lhs.type == INSTANCE) {
                    auto it = lhs.members.find("to_string");
                    if (it != lhs.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 0) {
                                Scope layer;
                                layer["this"] = lhs.box_location;
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                converted = fn.vm.run().getStr();
                            } else {
                                error("run-time error: invalid type for operation  object: " + lhs.toString());
                            }
                        } else {
                            error("run-time error: invalid type for operation  object: " + lhs.toString());
                        }
                    } else {
                        error("run-time error: invalid type for operation  object: " + lhs.toString());
                    }
                } else {
                    converted = lhs.toString();
                    if (converted.at(0) == '"') {
                        converted = lhs.str;
                    }
                }
                stack.push(strValue(converted + rhs.getStr()));
            } else 
                stack.push(intValue(lhs.getInt() % rhs.getInt()));
        } else if (OP == OP_DIVIDE) {
            SIDES();
            if (lhs.type == FLOAT && ( rhs.type == FLOAT || rhs.type == INTIGER )) {
                if (rhs.type == FLOAT) {
                    stack.push(floatValue( lhs.getFloat() / rhs.getFloat() ));
                } else if (rhs.type == INTIGER) {
                    stack.push(floatValue( lhs.getFloat() / rhs.getInt() ));
                } else {
                    rhs.error("invalid type for operation"); 
                }
            } else if (lhs.type == INTIGER && ( rhs.type == FLOAT || rhs.type == INTIGER )) {
                if (rhs.type == FLOAT) {
                    stack.push(floatValue( lhs.getInt() / rhs.getFloat() ));
                } else if (rhs.type == INTIGER) {
                    stack.push(floatValue( lhs.getInt() / rhs.getInt() ));
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
                    auto it = top.members.find("operator_divide");
                    if (it != top.members.end()) {
                        if (heap.get(it->second).type == FUNCTION) {
                            auto fn = heap.fn_get(heap.get(it->second).getFun());
                            if (fn.args.size() == 1) {
                                Scope layer;
                                layer["this"] = top.box_location;
                                layer[fn.args[0]] = heap.add(arg);
                                fn.vm.scopes.push_back(layer);
                                fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                                stack.push(fn.vm.run());
                                continue;
                            }
                        }
                    }
                    lhs.error("expected either an instance or a number.");
                } else {
                    lhs.error("expected either an instance or a number.");
                }
            }

        } else if (OP == OP_MORE) {
            bBASIC_OPERATION(>);
        } else if (OP == OP_MORE_EQ) {
            bBASIC_OPERATION(>=);
        } else if (OP == OP_LESS) {
            bBASIC_OPERATION(<);
        } else if (OP == OP_LESS_EQ) {
            bBASIC_OPERATION(<=);

        } else if (OP == OP_EQUALITY) {
            SIDES();
            switch (lhs.type) {
                case STRING:
                    stack.push(boolValue(lhs.getStr() == rhs.getStr()));
                    break;
                case INTIGER:
                    stack.push(boolValue(lhs.getInt() == rhs.getInt()));
                    break;
                case FLOAT:
                    stack.push(boolValue(lhs.getFloat() == rhs.getInt()));
                    break;
                case BOOLEAN:
                    stack.push(boolValue(lhs.getBool() == rhs.getBool()));
                    break;
                default:
                    error("run-time error: operator '==' does not support that type.");
            }
        } else if (OP == OP_NOT_EQUAL) {
            SIDES();
            switch (lhs.type) {
                case STRING:
                    stack.push(boolValue(lhs.getStr() != rhs.getStr()));
                    break;
                case INTIGER:
                    stack.push(boolValue(lhs.getInt() != rhs.getInt()));
                    break;
                case FLOAT:
                    stack.push(boolValue(lhs.getFloat() != rhs.getInt()));
                    break;
                case BOOLEAN:
                    stack.push(boolValue(lhs.getBool() != rhs.getBool()));
                    break;
                default:
                    error("run-time error: operator '!=' does not support that type.");
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

        } else if (OP == OP_COPY) {
            TOP();
            top.box_location = -1;
            stack.push(top);
        } else if (OP == OP_CALL_FN) {
            std::vector<Value> args;
            for (int i = 0; i < INSTRUCTION.i; i++) {
                TOP();
                args.insert(args.begin(), top);
            }
            TOP();
            int fn_loc = top.getFun();
            Function f = heap.fn_get(fn_loc);
            if (f.args.size() != args.size()) error("run-time error: expected " + std::to_string(f.args.size()) + " arguments, found " + std::to_string(args.size()));
            f.vm.scopes = scopes;
            f.vm.scopes.push_back(Scope());
            f.vm.templates.push_back(std::map<std::string, ClassTemplate>());
            for (int i = 0; i < f.args.size(); i++) {
                f.vm.scopes.back()[f.args[i]] = heap.add(args[i]);
            }
            if (top.home_location != -1) {
                f.vm.scopes.back()["this"] = top.home_location;
            }
            stack.push(f.vm.run());
            for (int i = 0; i < scopes.size(); i++) scopes[i] = f.vm.scopes[i];

        } else if (OP == OP_PRINT_POP) {
            TOP();
            if (top.type == INSTANCE) {
                auto it = top.members.find("to_string");
                if (it != top.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 0) {
                            Scope layer;
                            layer["this"] = top.box_location;
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            std::cout << fn.vm.run().getStr();
                            continue;
                        }
                    }
                }
            }

            std::string s = top.toString();
            if (top.type == STRING)
                std::cout << s.substr(1, s.length()-2);
            else
                std::cout << s;
            
        } else if (OP == OP_RETURN_POP) {
            TOP();
            return top;
        }

        else if (OP == OP_JUMP) {
            ip += INSTRUCTION.i;
        } else if (OP == OP_JUMP_FALSE) {
            TOP();
            if (!top.getBool()) {
                ip += INSTRUCTION.i;
            }
        } else if (OP == OP_ERROR) {
            error("OP_ERROR found.");
        }

        else if (OP == OP_GOTO_LABEL) {
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
        }

        else if (OP == OP_GET_MEMBER) {
            SIDES();
            if (lhs.members.find(rhs.getIden()) != lhs.members.end())
                stack.push(heap.get(lhs.members[rhs.getIden()]));
            else {
                std::string message = "run-time error: object has no member " + rhs.getIden() + ". did you mean: [";
                for (auto it = lhs.members.begin(); it != lhs.members.end(); it++) {
                    message += it->first + ", ";
                }
                if (message.back() == ' ') {
                    message.pop_back();
                    message.pop_back();
                }
                error(message + "]");
            }
        }

        else if (OP == OP_INDEX) {
            SIDES();
            if (lhs.type == STRING) {
                if (lhs.getStr().length() <= rhs.getInt()) error("run-time error: index " + std::to_string(rhs.getInt()) + " out of range.");
                else stack.push(strValue(std::string(1, lhs.getStr().at(rhs.getInt()))));
            } else if (lhs.type == LIST) {
                stack.push(heap.get(lhs.list_locations[rhs.getInt()]));
            } else if (lhs.type == INSTANCE) {
                auto it = lhs.members.find("index");
                if (it != lhs.members.end()) {
                    if (heap.get(it->second).type == FUNCTION) {
                        auto fn = heap.fn_get(heap.get(it->second).getFun());
                        if (fn.args.size() == 1) {
                            Scope layer;
                            layer["this"] = lhs.box_location;
                            layer[fn.args[0]] = heap.add(rhs);
                            fn.vm.scopes.push_back(layer);
                            fn.vm.templates.push_back(std::map<std::string, ClassTemplate>());
                            stack.push(fn.vm.run());
                        } else {
                            error("run-time error: cannot index a non-string, non-list object : " + lhs.toString());
                        }
                    }
                } else {
                    error("run-time error: cannot index a non-string, non-list object : " + lhs.toString());
                }
            } else {
                error("run-time error: cannot index a non-string, non-list object : " + lhs.toString());
            }
        }

        else if (OP == OP_DECL_CLASS) {
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
            ClassTemplate templt;
            bool found = false;
            for (int s = templates.size()-1; s >= 0; s--) {
                auto it = templates[s].find(opcode[ip].lexeme);
                if (it != templates[s].end()) {
                    found = true;
                    templt = it->second;
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

            stack.push(instanceValue(templt));
        } else if (OP == OP_CREATE_LIST) {
            Locations list;
            for (int i = 0; i < opcode[ip].i; i++) {
                TOP();
                list.push_back(heap.add(top));
            }
            std::reverse(list.begin(), list.end());
            stack.push(listValue(list));
        }

        /*else if (OP == OP_OK_FN) {
            TOP();
            if (top.type == NIL) {
                error("run-time error: found ")
            } else {
                stack.push(top);
            }
        }*/
    }
    #undef OP
    #undef INSTUCTION
    //std::cout << "\nsucess" << std::endl;
    return intValue(0);
}

#endif