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
    EQUALS,
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

    IDENTIFIER,
    NUMBER,
    STRING,
    
    NIL,
    TRUE,
    FALSE,
    WHILE,
    FOR,
    SET,
    CLASS,
    FN,
    INST,


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
                else if (lexeme == "null") ADD_TOKEN(NIL);
                else if (lexeme == "for") ADD_TOKEN(FOR);
                else if (lexeme == "while") ADD_TOKEN(WHILE);
                else if (lexeme == "false") ADD_TOKEN(FALSE);
                else if (lexeme == "fn") ADD_TOKEN(FN);
                else if (lexeme == "new") ADD_TOKEN(INST);
                else ADD_TOKEN(IDENTIFIER);
            } else if (text::digit(lexeme[0])) { // parse number
                while (text::digit(SRC)) {
                    lexeme += _get();
                }
                if (SRC == '.') {
                    lexeme += _get();
                    while (text::digit(SRC)) {
                        lexeme += _get();
                    }
                }
                ADD_TOKEN(NUMBER);
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
                ADD_TOKEN(STRING);
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
                            ADD_TOKEN(EQUALS);
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
        lines.push_back(std::string());
    }    

    Token next_token() {
        if (!cache.empty) {
            cache.empty = true;
            return cache.stored;
        } else {
            return _parse_next();
        }
    }

    Token peek_next_token() {
        if (cache.empty) {
            cache.empty = false;
            cache.stored = _parse_next();
        }
        return cache.stored;
    }

    inline bool done() {
        return source.length() == 0;
    }

    void error(Token error_token, std::string message) {
        for (int i = 0; i < lines.size(); i++) {
            lines[i] = std::to_string(i) + " | " + lines[i];
        }
        std::cerr << message << std::endl << *(error_token.context.line);
        exit(1);
    }
};