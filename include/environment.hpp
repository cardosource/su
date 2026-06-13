#pragma once
#include <string>
#include <unordered_map>
#include <any>
#include <memory>
#include "debug.hpp"
#include "token.hpp"

 
struct Binding {
  std::shared_ptr<std::any> value;  // shared — graph sharing
  bool immutable = true;            // immutable binding por padrão
};

class Env: public std::enable_shared_from_this<Env> {
private:
  std::shared_ptr<Env> enclosing;
  std::unordered_map<std::string, Binding> bindings;

public:
  Env();
  Env(std::shared_ptr<Env> enclosing);

 
  void define(const std::string& name, std::any value, int line = 0);
 
  void defineShared(const std::string& name, std::shared_ptr<std::any> ptr, int line = 0);

  void assign(const Token& name, std::any value);

  std::any get(const Token& name);

  std::shared_ptr<std::any> getPtr(const Token& name);
  std::shared_ptr<std::any> getPtrByName(const std::string& name);
};
