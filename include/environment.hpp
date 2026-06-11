#pragma once
#include <string>
#include <unordered_map>
#include <any>
#include <memory>
#include "debug.hpp"
#include "token.hpp"

class Env: public std::enable_shared_from_this<Env> {
private:
  std::shared_ptr<Env> enclosing;
  std::unordered_map<std::string, std::any> values;
  
public:
  Env();
  Env(std::shared_ptr<Env> enclosing);
  void define(const std::string& name, std::any value);
  void assign(const Token& name, std::any value);
  std::any get(const Token& name);
};
