#ifndef ERROR_HPP
#define ERROR_HPP

#include <string>
#include <iostream>

inline void error(std::string msg, int code = 1) {
    std::cerr << msg;
    std::cerr.flush();
    exit(code);
}

/*
template <typename T> struct Result {
    T container;
    bool ok;

    T unwrap(msg = "unwrap error") {
        if (result) return container;
        else error(msg)
    }
};

template <typename N> Result<N> Ok(N item) {
    Result r;
    r.container = N;
    ok = true;
    return r;
}

template <typename N> Result<N> Err(N item) {
    Result r;
    ok = false;
    return r;
}
*/

#endif