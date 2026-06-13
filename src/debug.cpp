#include "debug.hpp"
#include "runtimeError.hpp"
#include "token.hpp"
#include <iostream>
#include <string>
#include <unordered_map>

static const std::unordered_map<std::string, std::string> explanations = {
  // comando desconhecido
  {"is not a recognized command in .su",
   "Check the .su documentation for valid commands. Known commands: proclaim(), if, while, for."},

  // parser — fim de statement
  {"Esperado ';' após valor.",
   "Every statement must end with ';' — discipline is the foundation of order."},
  {"Esperado ';' após expressão.",
   "Every statement must end with ';' — discipline is the foundation of order."},

  // parser — parênteses e blocos
  {"Expected ')' after expression.",
   "An open '(' was never closed. Every opening demands a closing."},
  {"Expected ')' after typeOf operand.",
   "typeOf() requires closing ')' — syntax serves the collective, not the individual."},
  {"Expected '(' after 'typeOf'.",
   "typeOf must be followed by '(' — the form must be respected."},
  {"Expected ')' after if condition.",
   "The condition of 'if' must be enclosed in '(' and ')'."},
  {"Expected '(' after 'if'.",
   "The condition of 'if' must begin with '(' — structure precedes freedom."},
  {"Expected '}' after block.",
   "A block opened with '{' was never closed with '}'. Every revolution needs an end."},

  // parser — expressões e variáveis
  {"Expected expression.",
   "An expression was expected here but was not found. The compiler cannot proceed without substance."},
  {"Esperado nome da variável.",
   "A variable name was expected. Identity is required before assignment."},
  {"Esperado '=' após nome da variável.",
   "After the variable name, '=' is required to assign a value."},
  {"invalida assignment target",
   "The left side of '=' must be a variable. You cannot assign to an expression."},

  // runtime — aritmética
  {"Divisão por zero.",
   "Production cannot be divided among zero workers."},
  {"Operand deve ser um número.",
   "This operation requires a number. Check the type with typeOf()."},
  {"Operands devem ser números.",
   "Both sides of this operation must be numbers. Check types with typeOf()."},
  {"Overflow: soma de inteiros excede o limite.",
   "The sum exceeds the maximum integer. Consider using smaller values."},
  {"Overflow: resultado float é infinito.",
   "The float result is infinite — the means of production have been exhausted."},
};

// Mensagens com prefixo dinâmico (nome de variável embutido)
static const std::unordered_map<std::string, std::string> prefixExplanations = {
  {"Variável não referenciada:",
   "This variable was never declared. Use 'name = value;' before referencing it."},
  {"Variável não definida:",
   "Cannot assign to a variable that was never declared. Declare it first."},
  {"Unreferenced binding:",
   "This binding was never declared. Use 'name = value;' before referencing it."},
  {"Undefined binding:",
   "This binding was never declared. Declare it before assigning."},
  {"The binding",
   "Each name belongs to one value, forever — immutable binding. Declare a new name instead."},
};

static std::string explain(const std::string& message){

  auto it = explanations.find(message);
  if(it != explanations.end())
    return "\n  -> " + it->second;


  for(const auto& [prefix, explanation] : prefixExplanations){
    if(message.rfind(prefix, 0) == 0)
      return "\n  -> " + explanation;
  }

  return "";
}



void Debug::report(int line, const std::string& where, const std::string& message){
  hardError = true;
  std::cerr << "\n[ERROR on line " << line << "]"
            << " Deviation detected" << where << ":\n"
            << "  ✖ " << message
            << explain(message)
            << "\n";
}

void Debug::error(int line, const std::string& message){
  report(line, "", message);
}

void Debug::error(Token token, const std::string& message){
  if(token.type == TokenType::MY_EOF){
    report(token.line, " at end of file", message);
  } else {
    report(token.line, " near '" + token.lexeme + "'", message);
  }
}

void Debug::runtimeError(RuntimeError error){
  std::cerr << "\n[FATAL ERROR on the line " << error.token.line << "] "
            << "Contradiction during execution:\n"
            << "  ✖ " << error.what()
            << explain(error.what())
            << "\n";
}
