#pragma once
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include "scanner.hpp"
#include "expr.hpp"
#include "debug.hpp"
#include "visitor.hpp"
#include "stmt.hpp"

class Parser{
private:
  struct ParseError: public std::runtime_error{
    using std::runtime_error::runtime_error;
  };

  const std::vector<Token> tokens;
  int current = 0;

  ParseError error(const Token&, const std::string&);
  void synchronize();
  bool isAtEnd();
  bool check(const TokenType&);

  template<class...T>
  bool match(T...types);

  Token previous();
  Token peek();
  Token advance();
  Token consume(const TokenType&, const std::string&);

  std::shared_ptr<Expr> expression();
  std::shared_ptr<Expr> equality();
  std::shared_ptr<Expr> comparison();
  std::shared_ptr<Expr> term();
  std::shared_ptr<Expr> factor();
  std::shared_ptr<Expr> unary();
  std::shared_ptr<Expr> postfix();
  std::shared_ptr<Expr> primary();
  std::shared_ptr<Expr> assignment();
  std::shared_ptr<Expr> logicalOr();
  std::shared_ptr<Expr> logicalAnd();
  std::shared_ptr<Expr> parseStringInterp(const std::string& raw, int line);

  std::shared_ptr<Statement::Stmt> statement();
  std::shared_ptr<Statement::Stmt> proclaimStatement();
  std::shared_ptr<Statement::Stmt> expressionStatement();
  std::shared_ptr<Statement::Stmt> assignmentStatement();
  std::shared_ptr<Statement::Stmt> compoundAssignStatement();
  std::shared_ptr<Statement::Stmt> declaration();
  std::vector<std::shared_ptr<Statement::Stmt>> block();
  std::shared_ptr<Statement::Stmt> IfStatement();
  std::shared_ptr<Statement::Stmt> whileStatement();
  std::shared_ptr<Statement::Stmt> forStatement();

public:
  Parser(const std::vector<Token>&);
  std::vector<std::shared_ptr<Statement::Stmt>> parser();
};
