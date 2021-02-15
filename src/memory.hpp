#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "value.hpp"
#include "function.hpp"

#include <map>

struct VirtualMemory {
    std::map<size_t, Value> memory;
    size_t current;

    std::map<size_t, Function> fn_memory;
    size_t fn_current;

    void init() {
        // standard library loaded directly into memory
        current = 0;
        fn_current = 0;
    }

    int add(Value new_value) {
        memory[current] = new_value;
        memory[current].box_location = current;
        current++;
        return current-1;
    }
    void change(size_t pos, Value new_value) {
        memory[pos] = new_value;
        memory[pos].box_location = pos;
    }
    void dump(size_t pos) {
        memory.erase(pos);
    }
    Value get(size_t pos) {
        return memory[pos];
    }

    int fn_add(Function new_value) {
        fn_memory[fn_current] = new_value;
        fn_current++;
        return fn_current-1;
    }
    void fn_change(size_t pos, Function new_value) {
        fn_memory[pos] = new_value;
    }
    void fn_dump(size_t pos) {
        fn_memory.erase(pos);
    }
    Function fn_get(size_t pos) {
        return fn_memory[pos];
    }

} heap;

std::string Value::toString() {
    switch (type) {
        case INTIGER:
            return std::to_string(intiger);
        case FLOAT:
            return std::to_string(floating);
        case BOOLEAN:
            return std::to_string(boolean);
        case NIL:
            return "null";
        case STRING:
            return "\"" + str + "\"";
        case LIST: {
            std::string final = "[";
            for (int i = 0; i < list_locations.size(); i++) {
                final += heap.get(list_locations[i]).toString() + ", ";
            }
            if (final.length() > 1) {
                final.pop_back();
                final.pop_back();
            }
            return final + "]";
        }
        case IDENTIFIER: {
            return str;
        }
        case FUNCTION: {
            std::string final = "fn(";
            for (int i = 0; i < heap.fn_get(fn).args.size(); i++) {
                final += heap.fn_get(fn).args[i] + ", ";
            }
            if (final.length() > 3) {
                final.pop_back();
                final.pop_back();
            }
            return final + ")";
        }
    }
    return "";
}

Value instanceValue(ClassTemplate tmplt) {
    Value instance;
    instance.type = INSTANCE;
    int loc = heap.add(instance);
    for (auto it = tmplt.members.begin(); it != tmplt.members.end(); it++) {
        Value v = it->second;
        v.c_lass = loc;
        heap.memory[loc].members[it->first] = heap.add(v);
    }
    return heap.memory[loc];
}

#endif