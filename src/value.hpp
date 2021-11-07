#ifndef VALUE_HPP
#define VALUE_HPP

#include <string>
#include <iostream>
#include <vector>
#include <map>

typedef std::vector<size_t> Locations;

enum PrimType {
    INTIGER,
    FLOAT,
    STRING,
    BOOLEAN,
    NIL,
    BYTE,
    POINTER,
    LIST,
    FUNCTION,
    IDENTIFIER,
    INSTANCE,

    STL_CALL,
};

enum LibraryCall {
    LIST_TO_STRING,
    LIST_JOIN,
};

struct Value {
    union {
        LibraryCall stl_call;
        int intiger;
        float floating;
        bool boolean;
        size_t pointer;
        unsigned char byte;
    };
    std::string str;
    int box_location;
    int home_location;
    size_t fn;
    std::map<std::string, size_t> members;
    Locations list_locations;

    PrimType type;

    inline Value& locate_base(int l) {
        home_location = l;
        return *this;
    }

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
    inline unsigned char &getByte() {
        if (type == BYTE) return byte;
        else error("expected a byte");
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

    inline size_t &getFun() {
        if (type == FUNCTION) return fn;
        else error("expected a function");
    }

    inline LibraryCall &getSTL() {
        if (type == STL_CALL) return stl_call;
        else error("expected a library object");
    }

    inline int getBoxLoc() {
        if (box_location == -1) {
            error("expected a boxed value");
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
    v.home_location = -1;
    v.getInt() = i;
    return v;
}

Value floatValue(float f) {
    Value v;
    v.type = FLOAT;
    v.box_location = -1;
    v.getFloat() = f;
    v.home_location = -1;
    return v;
}

Value strValue(std::string s) {
    Value v;
    v.type = STRING;
    v.box_location = -1;
    v.home_location = -1;
    v.getStr() = s;
    return v;
}

Value boolValue(bool b) {
    Value v;
    v.type = BOOLEAN;
    v.home_location = -1;
    v.getBool() = b;
    v.box_location = -1;
    return v;
}

Value stlValue(LibraryCall lc) {
    Value v;
    v.type = STL_CALL;
    v.home_location = -1;
    v.getSTL() = lc;
    v.box_location = -1;
    return v;
}

Value byteValue(unsigned char b) {
    Value v;
    v.type = BYTE;
    v.home_location = -1;
    v.getByte() = b;
    v.box_location = -1;
    return v;
}

Value nullValue() {
    Value v;
    v.type = NIL;
    v.box_location = -1;
    v.home_location = -1;
    return v;
}

Value ptrValue(size_t loc) {
    Value v;
    v.type = POINTER;
    v.box_location = -1;
    v.home_location = -1;
    v.pointer = loc;
    return v;
}

Value idenValue(std::string id) {
    Value v;
    v.type = IDENTIFIER;
    v.box_location = -1;
    v.home_location = -1;
    v.str = id;
    return v;
}

Value listValue(Locations l) {
    Value v;
    v.type = LIST;
    v.box_location = -1;
    v.home_location = -1;
    v.getList() = l;
    return v;
}

Value funValue(size_t fn) {
    Value v;
    v.type = FUNCTION;
    v.box_location = -1;
    v.home_location = -1;
    v.getFun() = fn;
    return v;
}

std::string trim(Value v) {
    if (v.type == STRING)
        return v.toString().substr(1, v.toString().length()-2);
    return v.toString();
}

#endif