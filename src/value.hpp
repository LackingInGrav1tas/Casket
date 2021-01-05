#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <vector>

typedef std::vector<size_t> Locations;

enum PrimType {
    INTIGER,
    FLOAT,
    STRING,
    BOOLEAN,
    NIL,
    POINTER,
    LIST,
    IDENTIFIER,
    INSTANCE
};

struct Value {
    int intiger;
    float floating;
    bool boolean;
    std::string str;
    size_t pointer;
    Locations list_locations;

    PrimType type;

    int &getInt() {
        if (type == INTIGER) return intiger;
        else exit(2);
    }
    float &getFloat() {
        if (type == FLOAT) return floating;
        else exit(2);
    }
    bool &getBool() {
        if (type == BOOLEAN) return boolean;
        else exit(2);
    }
    std::string &getIden() {
        if (type == IDENTIFIER) return str;
        else exit(2);
    }
    size_t &getPtr() {
        if (type == POINTER) return pointer;
        else exit(2);
    }
    std::string &getStr() {
        if (type == STRING) return str;
        else exit(2);
    }
    Locations &getList() {
        if (type == LIST) return list_locations;
        else exit(2);
    }

    std::string toString();

};

Value intValue(int i) {
    Value v;
    v.type = INTIGER;
    v.getInt() = i;
    return v;
}

Value floatValue(float f) {
    Value v;
    v.type = FLOAT;
    v.getFloat() = f;
    return v;
}

Value strValue(std::string s) {
    Value v;
    v.type = STRING;
    v.getStr() = s;
    return v;
}

Value boolValue(bool b) {
    Value v;
    v.type = BOOLEAN;
    v.getBool() = b;
    return v;
}

Value nullValue() {
    Value v;
    v.type = NIL;
    return v;
}

Value ptrValue(size_t loc) {
    Value v;
    v.type = POINTER;
    v.pointer = loc;
    return v;
}

Value idenValue(std::string id) {
    Value v;
    v.type = IDENTIFIER;
    v.str = id;
    return v;
}

Value listValue(Locations l) {
    Value v;
    v.type = LIST;
    v.getList() = l;
    return v;
}

#endif