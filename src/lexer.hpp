#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <string>
#include <iostream>

#define PRINT(s) std::cout << s << std::endl;

namespace text {
    inline bool alpha(char c) {
        return ('a' <= c) && (c <= 'z') || ('A' <= c) && (c <= 'Z') || c == '_';
    }

    inline bool digit(char c) {
        return ('0' <= c) && (c <= '9');
    }
}

enum TokenType {
    PLUS,
    MINUS,
    STAR,
    SLASH,
    AND,
    BIT_AND,
    OR,
    BIT_OR,
    ASSIGN,
    EQUALITY,
    NOT_EQUALITY,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    PLUS_PLUS,
    PLUS_EQUALS,
    MINUS_MINUS,
    MINUS_EQUALS,
    STAR_EQUALS,
    SLASH_EQUALS,
    AND_EQUALS,
    OR_EQUALS,
    BIT_AND_EQUALS,
    BIT_OR_EQUALS,
    PERCENT_EQUALS,
    PERCENT,
    DECIMAL,

    RIGHT_PAREN, // ()
    LEFT_PAREN,

    RIGHT_BRACKET, // {}
    LEFT_BRACKET,

    RIGHT_BRACE, // []
    LEFT_BRACE,

    COLON,
    SEMICOLON,
    COMMA,
    CARROT_EQUALS,
    CARROT,
    EXCLAMATION_MARK,

    T_IDENTIFIER,
    T_NUMBER,
    T_STRING,
    T_BYTE,
    
    T_NIL,
    TRUE,
    FALSE,
    WHILE,
    FOR,
    SET,
    CLASS,
    FN,
    INST,
    ENUM,


    ERROR
};

struct Context {
    std::string *line;
};

struct Token {
    std::string value;
    TokenType type;
    Context context;

    bool is(std::vector<std::string> check) {
        for (int i = 0; i < check.size(); i++) {
            if (check[i] == value) return true;
        }
        return false;
    }

    bool is(std::vector<TokenType> check) {
        for (int i = 0; i < check.size(); i++) {
            if (check[i] == type) return true;
        }
        return false;
    }
};

Token errorToken() {
    Token t;
    t.type = ERROR;
    return t;
}

struct TokenCache {
    bool empty;
    Token stored;
};

class Lexer {
private:
    std::vector<std::string> lines;
    std::string source;
    TokenCache cache;
    TokenCache last;

    char _get() {
        char c = source[0];
        lines.back() += c;
        source.erase(source.begin());
        return c;
    }

    Token _parse_next() {
        #define SRC source[0]
        #define ADD_TOKEN(T) do { \
            Token token; \
            token.type = T; \
            token.value = lexeme; \
            token.context.line = &lines.back(); \
            return token; \
        } while (0)

        while (1) {
            std::string lexeme(1, _get());
            if (text::alpha(lexeme[0])) { // parse identifier
                while (text::alpha(SRC) || text::digit(SRC)) {
                    lexeme += _get();
                }
                if (lexeme == "true") ADD_TOKEN(TRUE);
                else if (lexeme == "false") ADD_TOKEN(FALSE);
                else if (lexeme == "null") ADD_TOKEN(T_NIL);
                else if (lexeme == "for") ADD_TOKEN(FOR);
                else if (lexeme == "while") ADD_TOKEN(WHILE);
                else if (lexeme == "false") ADD_TOKEN(FALSE);
                else if (lexeme == "fn") ADD_TOKEN(FN);
                else if (lexeme == "new") ADD_TOKEN(INST);
                else if (lexeme == "set") ADD_TOKEN(SET);
                else if (lexeme == "enum") ADD_TOKEN(ENUM);
                else ADD_TOKEN(T_IDENTIFIER);
            } else if (text::digit(lexeme[0])) { // parse number
                if (lexeme[0] == '0' && source[0] == 'b') { // byte
                    _get();
                    lexeme = "";
                    int i = 0;
                    for (; source[0] == '0' || source[0] == '1'; i++) {
                        lexeme += _get();
                    }
                    if (i != 8) {
                        std::cerr << "lexing error: byte is not 8 bits" << std::endl;
                        exit(1);
                    }
                    ADD_TOKEN(T_BYTE);
                } else { // number
                    while (text::digit(SRC)) {
                        lexeme += _get();
                    }
                    if (SRC == '.') {
                        lexeme += _get();
                        while (text::digit(SRC)) {
                            lexeme += _get();
                        }
                    }
                    ADD_TOKEN(T_NUMBER);
                }
            } else if (lexeme[0] == '"') { // parse string
                while (true) {
                    if (SRC == '"') {
                        break;
                    } else if (SRC == '\\') {
                        _get();
                        char escape = _get();
                        switch (escape) {
                            case 'n': lexeme += "\n"; break;
                            case 'r': lexeme += "\r"; break;
                            case 't': lexeme += "\t"; break;
                            case 'b': lexeme += "\b"; break;
                            case 'f': lexeme += "\f"; break;
                            case 'v': lexeme += "\v"; break;
                            case '0': lexeme += "\0"; break;

                            case '\\':
                            case '\'':
                            case '"': lexeme += escape; break;

                            default: {
                                std::cerr << "lexing error: '" << escape << "' is not a valid escape character." << std::endl;
                                exit(1);
                            }
                        }
                    } else if (SRC == '\n') {
                        lines.push_back(std::string());
                        lexeme += _get();
                    } else {
                        lexeme += _get();
                    }
                }
                lexeme += _get();
                lexeme = lexeme.substr(1,lexeme.length()-2);
                ADD_TOKEN(T_STRING);
            } else if (lexeme[0] == ' ' || lexeme[0] == '\t' || lexeme[0] == '\r') {
                lexeme.pop_back();
            } else if (lexeme[0] == '\n') {
                lines.push_back(std::string());
                lexeme.pop_back();
            } else {
                #define NEXT(c) (SRC == c)
                switch (lexeme[0]) {
                    case '+': {
                        if (NEXT('+')) {
                            lexeme += _get();
                            ADD_TOKEN(PLUS_PLUS);
                        } else if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(PLUS_EQUALS);
                        } else {
                            ADD_TOKEN(PLUS);
                        }
                    }
                    case '-': {
                        if (NEXT('-')) {
                            lexeme += _get();
                            ADD_TOKEN(MINUS_MINUS);
                        } else if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(MINUS_EQUALS);
                        } else {
                            ADD_TOKEN(MINUS);
                        }
                    }
                    case '*': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(STAR_EQUALS);
                        } else {
                            ADD_TOKEN(STAR);
                        }
                    }
                    case '/': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(SLASH_EQUALS);
                        } else {
                            ADD_TOKEN(SLASH);
                        }
                    }
                    case '%': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(PERCENT_EQUALS);
                        } else {
                            ADD_TOKEN(PERCENT);
                        }
                    }
                    case '!': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(NOT_EQUALITY);
                        } else {
                            ADD_TOKEN(EXCLAMATION_MARK);
                        }
                    }
                    case '^': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(CARROT_EQUALS);
                        } else {
                            ADD_TOKEN(CARROT);
                        }
                    }
                    case '|': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(BIT_OR_EQUALS);
                        } else if (NEXT('|')) {
                            _get();
                            if (source[1] == '=') {
                                _get();
                                ADD_TOKEN(OR_EQUALS);
                            }
                            else ADD_TOKEN(OR);
                        } else {
                            ADD_TOKEN(BIT_OR);
                        }
                    }
                    case '&': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(BIT_AND_EQUALS);
                        } else if (NEXT('&')) {
                            _get();
                            if (source[1] == '=') {
                                _get();
                                ADD_TOKEN(AND_EQUALS);
                            }
                            else ADD_TOKEN(AND);
                        } else {
                            ADD_TOKEN(BIT_AND);
                        }
                    }
                    case '=': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(EQUALITY);
                        } else {
                            ADD_TOKEN(ASSIGN);
                        }
                    }
                    case '<': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(LESS_EQUAL);
                        } else {
                            ADD_TOKEN(LESS);
                        }
                    }
                    case '>': {
                        if (NEXT('=')) {
                            lexeme += _get();
                            ADD_TOKEN(GREATER_EQUAL);
                        } else {
                            ADD_TOKEN(GREATER);
                        }
                    }
                    case '(': ADD_TOKEN(LEFT_PAREN);
                    case ')': ADD_TOKEN(RIGHT_PAREN);
                    case '{': ADD_TOKEN(LEFT_BRACKET);
                    case '}': ADD_TOKEN(RIGHT_BRACKET);
                    case '[': ADD_TOKEN(LEFT_BRACE);
                    case ']': ADD_TOKEN(RIGHT_BRACE);
                    case ';': ADD_TOKEN(SEMICOLON);
                    case ':': ADD_TOKEN(COLON);
                    case ',': ADD_TOKEN(COMMA);
                    case '.': ADD_TOKEN(DECIMAL);
                    default: {
                        std::cerr << "unexpected character '" << lexeme[0] << "'" << std::endl;
                        exit(1);
                    }
                }
                #undef NEXT
            }
        }
        #undef C
    }
public:
    Lexer(std::string s) {
        source = s;
        cache.empty = true;
        last.empty = true;
        lines.push_back(std::string());
    }    

    Token next_token() {
        if (!cache.empty) {
            cache.empty = true;
            last.empty = false;
            last.stored = cache.stored;
            return last.stored;
        } else {
            last.empty = false;
            last.stored = _parse_next();
            return last.stored;
        }
    }

    Token peek_next_token() {
        if (cache.empty) {
            cache.empty = false;
            cache.stored = _parse_next();
        }
        return cache.stored;
    }

    Token last_token() {
        if (!last.empty) {
            return last.stored;
        } else {
            std::cerr << "no 'last' token exists" << std::endl;
        }
    }

    inline bool done() {
        return source.length() == 0;
    }

    void error(Token error_token, std::string message) {
        for (int i = 0; i < lines.size(); i++) {
            lines[i] = std::to_string(i+1) + " | " + lines[i];
        }
        std::cerr << message << " [\"" << error_token.value << "\" " << error_token.type << "]" << std::endl << *(error_token.context.line) << std::endl;
        for (int i = 0; i < (*(error_token.context.line)).length() - error_token.value.length(); i++)
            std::cerr << " ";
        for (int i = 0; i < error_token.value.length(); i++)
            std::cerr << "^";
        exit(1);
    }
};

#endif