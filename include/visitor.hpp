#pragma once
#include <algorithm>
#include <any>
#include <memory>

struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Variable;
struct Assign;
struct Logical;
struct TypeOf;
struct CompoundAssign;
struct PreIncDec;
struct PostIncDec;
struct StringInterp;

struct ExprVisitor{
  virtual std::any visitBinaryExpr(std::shared_ptr<Binary> expr) = 0;
  virtual std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) = 0;
  virtual std::any visitLiteralExpr(std::shared_ptr<Literal> expr) = 0;
  virtual std::any visitUnaryExpr(std::shared_ptr<Unary> expr) = 0;
  virtual std::any visitVariable(std::shared_ptr<Variable> expr) = 0;
  virtual std::any visitAssignExpr(std::shared_ptr<Assign> expr) = 0;
  virtual std::any visitLogicalExpr(std::shared_ptr<Logical> expr) = 0;
  virtual std::any visitTypeOfExpr(std::shared_ptr<TypeOf> expr) = 0;
  virtual std::any visitCompoundAssign(std::shared_ptr<CompoundAssign> expr) = 0;
  virtual std::any visitPreIncDec(std::shared_ptr<PreIncDec> expr) = 0;
  virtual std::any visitPostIncDec(std::shared_ptr<PostIncDec> expr) = 0;
  virtual std::any visitStringInterp(std::shared_ptr<StringInterp> expr) = 0;
  virtual ~ExprVisitor() = default;
};

struct Expr{
  virtual std::any accept(ExprVisitor &visitor) = 0;
  virtual ~Expr() = default;
};

namespace Statement{
struct Expression;
struct Proclaim;
struct Assign;
struct Var;
struct Block;
struct If;
struct While;

struct StmtVisitor {
 virtual std::any visitExpressionStmt(std::shared_ptr<Expression> stmt) = 0;
 virtual std::any visitProclaimStmt(std::shared_ptr<Proclaim> stmt) = 0;
 virtual std::any visitAssignStmt(std::shared_ptr<Assign> stmt) = 0;
 virtual std::any visitVarStmt(std::shared_ptr<Var> stmt) = 0;
 virtual std::any visitBlockStmt(std::shared_ptr<Block> stmt) = 0;
 virtual std::any visitIfStmt(std::shared_ptr<If> stmt) = 0;
 virtual std::any visitWhileStmt(std::shared_ptr<While> stmt) = 0;
 virtual ~StmtVisitor() = default;
};

struct Stmt{
  virtual std::any accept(StmtVisitor& visitor) = 0;
  virtual ~Stmt() = default;
};
}
