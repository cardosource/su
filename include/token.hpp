#pragma once
#include <iostream>
#include <any>
#include <sstream>
#include <string>
#include "tokenType.hpp"

class Token {
public:
    TokenType type;
    std::string lexeme;
    std::any literal;
    int line;
    
    Token() : type(TokenType::MY_EOF), lexeme(""), literal{}, line(0) {}
    
    Token(TokenType type, std::string lexeme, std::any literal, int line);
    
    std::string toString();
};
