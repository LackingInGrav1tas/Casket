#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "value.hpp"

#include <map>

struct StorageMemory {
    std::map<size_t, Value> memory;
    size_t current;

    void init() {
        // standard library loaded directly into memory
        current = 0;
    }

    int add(Value new_value) {
        memory[current] = new_value;
        current++;
        return current-1;
    }
    void change(size_t pos, Value new_value) {
        memory[pos] = new_value;
    }
    void dump(size_t pos) {
        memory.erase(pos);
    }
    Value get(size_t pos) {
        return memory[pos];
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
            final.pop_back();
            final.pop_back();
            return final + "]";
        }
        case IDENTIFIER: {
            return str;
        }
    }
    return "";
}

#endif