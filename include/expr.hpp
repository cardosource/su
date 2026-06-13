#pragma once
#include <any>
#include <memory>
#include <utility>
#include <vector>
#include "visitor.hpp"
#include "token.hpp"

struct Binary : Expr, public std::enable_shared_from_this<Binary> {
  std::shared_ptr<Expr> left;
  Token oper;
  std::shared_ptr<Expr> right;
  Binary(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right);
  std::any accept(ExprVisitor &visitor) override;
};

struct Grouping : Expr, public std::enable_shared_from_this<Grouping> {
  std::shared_ptr<Expr> expression;
  Grouping(std::shared_ptr<Expr> expression);
  std::any accept(ExprVisitor &visitor) override;
};

struct Literal : Expr, public std::enable_shared_from_this<Literal> {
  std::any value;
  Literal(std::any value);
  std::any accept(ExprVisitor &visitor) override;
};

struct Unary : Expr, public std::enable_shared_from_this<Unary> {
  Token oper;
  std::shared_ptr<Expr> right;
  Unary(Token oper, std::shared_ptr<Expr> right);
  std::any accept(ExprVisitor &visitor) override;
};

struct Variable final : Expr, public std::enable_shared_from_this<Variable> {
  Token name;
  Variable(Token name);
  std::any accept(ExprVisitor &visitor) override;
};

struct Assign final : Expr, public std::enable_shared_from_this<Assign> {
  Token name;
  std::shared_ptr<Expr> value;
  Assign(Token name, std::shared_ptr<Expr> value);
  std::any accept(ExprVisitor &visitor) override;
};

struct Logical final : Expr, public std::enable_shared_from_this<Logical> {
  std::shared_ptr<Expr> left;
  Token oper;
  std::shared_ptr<Expr> right;
  Logical(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right);
  std::any accept(ExprVisitor &visitor) override;
};

struct TypeOf final : Expr, public std::enable_shared_from_this<TypeOf> {
  std::shared_ptr<Expr> operand;
  TypeOf(std::shared_ptr<Expr> operand);
  std::any accept(ExprVisitor &visitor) override;
};

 
struct IdOf final : Expr, public std::enable_shared_from_this<IdOf> {
  Token name;   
  IdOf(Token name);
  std::any accept(ExprVisitor &visitor) override;
};

 
struct CompoundAssign final : Expr, public std::enable_shared_from_this<CompoundAssign> {
  Token name;
  Token oper;
  std::shared_ptr<Expr> value;
  CompoundAssign(Token name, Token oper, std::shared_ptr<Expr> value);
  std::any accept(ExprVisitor &visitor) override;
};

 
struct PreIncDec final : Expr, public std::enable_shared_from_this<PreIncDec> {
  Token oper;
  Token name;
  PreIncDec(Token oper, Token name);
  std::any accept(ExprVisitor &visitor) override;
};

 
struct PostIncDec final : Expr, public std::enable_shared_from_this<PostIncDec> {
  Token name;
  Token oper;
  PostIncDec(Token name, Token oper);
  std::any accept(ExprVisitor &visitor) override;
};

 
struct StringInterp final : Expr, public std::enable_shared_from_this<StringInterp> {
  std::vector<std::shared_ptr<Expr>> parts;
  StringInterp(std::vector<std::shared_ptr<Expr>> parts);
  std::any accept(ExprVisitor &visitor) override;
};
