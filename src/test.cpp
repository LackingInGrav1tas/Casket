#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "lexer.hpp"

int main(int argc, char** argv) {
    std::cout << "START" << std::endl;
    std::ifstream file(argv[1]);
    std::stringstream buf;
    buf << file.rdbuf();

    Lexer lexer(buf.str());
    Token current;
    for (int i = 0; !lexer.done(); i++) {
        current = lexer.next_token();
        PRINT("(" << i << ") " << current.value << " " << current.type << " " << *(current.context.line) << " " << current.is((std::vector<TokenType>){IDENTIFIER}));
    }
    std::cout << "\nEND" << std::endl;
}