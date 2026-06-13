#include "environment.hpp"
#include "runtimeError.hpp"
#include <any>
#include <sstream>
#include <string>

Env::Env() : enclosing(nullptr) {}
Env::Env(std::shared_ptr<Env> enclosing) : enclosing(enclosing) {}


void Env::define(const std::string& name, std::any value, int line){
  if(bindings.count(name)){
    Token tok{TokenType::IDENTIFIER, name, std::any{}, line};
    throw RuntimeError(tok,
      "The binding '" + name + "' already belongs to the collective. "
      "In .su, all bindings are immutable — the revolution does not allow revisionism.");
  }
  bindings[name] = Binding{
    std::make_shared<std::any>(std::move(value)),
    true
  };
}


void Env::defineShared(const std::string& name, std::shared_ptr<std::any> ptr, int line){
  if(bindings.count(name)){
    Token tok{TokenType::IDENTIFIER, name, std::any{}, line};
    throw RuntimeError(tok,
      "The binding '" + name + "' already belongs to the collective. "
      "In .su, all bindings are immutable — the revolution does not allow revisionism.");
  }
  bindings[name] = Binding{ptr, true};
}

/* assign — usado por += -= etc vai criar novo binding com mesmo nome) 
 Na semântica funcional, += não muta o que acontece é  criar um novo valor
Permiti dentro do mesmo escopo para +=/-= */
void Env::assign(const Token& name, std::any value){
  auto it = bindings.find(name.lexeme);
  if(it != bindings.end()){
    it->second.value = std::make_shared<std::any>(std::move(value));
    return;
  }
  if(enclosing != nullptr){
    enclosing->assign(name, std::move(value));
    return;
  }
  throw RuntimeError(name, "Undefined binding: " + name.lexeme + ".");
}


std::any Env::get(const Token& name){
  auto it = bindings.find(name.lexeme);
  if(it != bindings.end()) return *(it->second.value);
  if(enclosing != nullptr) return enclosing->get(name);
  throw RuntimeError(name, "Unreferenced binding: " + name.lexeme + ".");
}

std::shared_ptr<std::any> Env::getPtr(const Token& name){
  auto it = bindings.find(name.lexeme);
  if(it != bindings.end()) return it->second.value;
  if(enclosing != nullptr) return enclosing->getPtr(name);
  throw RuntimeError(name, "Unreferenced binding: " + name.lexeme + ".");
}

std::shared_ptr<std::any> Env::getPtrByName(const std::string& name){
  auto it = bindings.find(name);
  if(it != bindings.end()) return it->second.value;
  if(enclosing != nullptr) return enclosing->getPtrByName(name);
  return nullptr;
}
