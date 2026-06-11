#include "environment.hpp"
#include "runtimeError.hpp"
#include <any>
#include <string>

Env::Env() : enclosing(nullptr) {}

Env::Env(std::shared_ptr<Env> enclosing) : enclosing(enclosing) {}

void Env::define(const std::string& name, std::any value){
  values[name] = std::move(value);
}

void Env::assign(const Token& name, std::any value){
  auto elem = values.find(name.lexeme);
  if(elem != values.end()){
    elem->second = std::move(value);
  } else if(enclosing != nullptr) {
    enclosing->assign(name, std::move(value));
  } else {
    throw RuntimeError(name, "Undefined variable: " + name.lexeme + ".");
  }
}

std::any Env::get(const Token& name){
  auto elem = values.find(name.lexeme);
  if(elem != values.end()){
    return elem->second;
  }
  
  if(enclosing != nullptr) {
    return enclosing->get(name);
  }
  
  throw RuntimeError(name, "Unreferenced variable: " + name.lexeme + ".");
}
