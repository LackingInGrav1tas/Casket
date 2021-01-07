#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include "vm.hpp"

#include <string>
#include <vector>

struct Function {
    std::vector<std::string> args;
    Machine vm;
};

#endif