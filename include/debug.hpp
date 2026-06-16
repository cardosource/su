#pragma once
#include <iostream>
#include <string>
#include "token.hpp"
#include "runtimeError.hpp"

class Debug{
  private:
    static void report(int, const std::string&, const std::string&);
  public:
    inline static bool hardError = false;
    inline static bool hardRuntimeError = false;
    static void error(int line, const std::string&);
    static void error(Token token, const std::string&);
    static void runtimeError(RuntimeError error);   
};
