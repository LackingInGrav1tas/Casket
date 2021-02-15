#ifndef CLASSES_HPP
#define CLASSES_HPP

#include "value.hpp"

#include <vector>
#include <map>

struct ClassTemplate {
    std::map<std::string, Value> members;
};

#endif