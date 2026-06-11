#pragma once
#include <stdexcept>
#include <string>
#include "token.hpp"

class RuntimeError : public std::runtime_error{
 public:
    const Token& token;
    RuntimeError(const Token& token,const std::string& message);

};
