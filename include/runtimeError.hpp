#pragma once
#include <stdexcept>
#include "token.hpp"

struct RuntimeError : public std::runtime_error {
  Token token;  // por VALOR — não referência, evita stack-use-after-return
  RuntimeError(Token token, const std::string& message)
    : std::runtime_error(message), token(std::move(token)) {}
};
