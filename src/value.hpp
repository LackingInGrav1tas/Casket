#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <iostream>
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
    int box_location;
    Locations list_locations;

    PrimType type;

    inline void error(std::string msg, int code = 1) {
        std::cerr << "run-time error: " << msg << "  object: " << toString() << std::endl;
        exit(code);
    }

    inline int &getInt() {
        if (type == INTIGER) return intiger;
        else error("expected an int");
    }
    inline float &getFloat() {
        if (type == FLOAT) return floating;
        else error("expected a float");
    }
    inline bool &getBool() {
        if (type == BOOLEAN) return boolean;
        else error("expected a bool");
    }
    inline std::string &getIden() {
        if (type == IDENTIFIER) return str;
        else error("expected an identifier");
    }
    inline size_t &getPtr() {
        if (type == POINTER) return pointer;
        else error("expected a pointer");
    }
    inline std::string &getStr() {
        if (type == STRING) return str;
        else error("expected a string");
    }
    inline Locations &getList() {
        if (type == LIST) return list_locations;
        else error("expected a list");
    }

    inline int getBoxLoc() {
        if (box_location == -1) {
            error("expected an identifier");
        } else {
            return box_location;
        }
    }

    Value edit(int i) {
        box_location = i;
        return *this;
    }

    std::string toString();

};

Value intValue(int i) {
    Value v;
    v.type = INTIGER;
    v.box_location = -1;
    v.getInt() = i;
    return v;
}

Value floatValue(float f) {
    Value v;
    v.type = FLOAT;
    v.box_location = -1;
    v.getFloat() = f;
    return v;
}

Value strValue(std::string s) {
    Value v;
    v.type = STRING;
    v.box_location = -1;
    v.getStr() = s;
    return v;
}

Value boolValue(bool b) {
    Value v;
    v.type = BOOLEAN;
    v.getBool() = b;
    v.box_location = -1;
    return v;
}

Value nullValue() {
    Value v;
    v.type = NIL;
    v.box_location = -1;
    return v;
}

Value ptrValue(size_t loc) {
    Value v;
    v.type = POINTER;
    v.box_location = -1;
    v.pointer = loc;
    return v;
}

Value idenValue(std::string id) {
    Value v;
    v.type = IDENTIFIER;
    v.box_location = -1;
    v.str = id;
    return v;
}

Value listValue(Locations l) {
    Value v;
    v.type = LIST;
    v.box_location = -1;
    v.getList() = l;
    return v;
}

#endif