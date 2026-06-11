#pragma once
#include "visitor.hpp"
#include "token.hpp"
#include <any>
#include <iterator>
#include <memory>
#include <vector>

namespace Statement{

  struct Expression : Stmt, public std::enable_shared_from_this<Expression>{
     const std::shared_ptr<Expr> expression;
     Expression(std::shared_ptr<Expr> expression);
     std::any accept(StmtVisitor& visitor) override;
  };

  struct Proclaim : Stmt, public std::enable_shared_from_this<Proclaim>{
     const std::shared_ptr<Expr> expression;
     Proclaim(std::shared_ptr<Expr> expression);
     std::any accept(StmtVisitor& visitor) override;
  };

  struct Assign : Stmt, public std::enable_shared_from_this<Assign>{
     Token name;
     std::shared_ptr<Expr> value;
     Assign(Token name, std::shared_ptr<Expr> value);
     std::any accept(StmtVisitor& visitor) override;
  };

  struct Var : Stmt, public std::enable_shared_from_this<Var> {
     Token name;
     std::shared_ptr<Expr> init;
     Var(Token name, std::shared_ptr<Expr> init);
     std::any accept(StmtVisitor &visitor) override;
  };

  // Implementações inline
  inline Expression::Expression(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  inline std::any Expression::accept(StmtVisitor &visitor){
    return visitor.visitExpressionStmt(shared_from_this());
  }

  inline Proclaim::Proclaim(std::shared_ptr<Expr> expression)
    : expression(std::move(expression)) {}

  inline std::any Proclaim::accept(StmtVisitor &visitor){
    return visitor.visitProclaimStmt(shared_from_this());
  }

  inline Assign::Assign(Token name, std::shared_ptr<Expr> value)
    : name(std::move(name)), value(std::move(value)) {}

  inline std::any Assign::accept(StmtVisitor &visitor){
    return visitor.visitAssignStmt(shared_from_this());
  }

  inline Var::Var(Token name, std::shared_ptr<Expr> init)
    : name(std::move(name)), init(std::move(init)) {}

  inline std::any Var::accept(StmtVisitor &visitor){
    return visitor.visitVarStmt(shared_from_this());
  }

  struct Block: Stmt, public std::enable_shared_from_this<Block>{
    std::vector<std::shared_ptr<Stmt>> statements;
    Block(std::vector<std::shared_ptr<Stmt>> statements);
    std::any accept(StmtVisitor &visitor) override;
  };

  struct If: Stmt, public std::enable_shared_from_this<If>{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;
    If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch);
    std::any accept(StmtVisitor &visitor) override;
  };
//} erro


struct While: Stmt, public std::enable_shared_from_this<While>{
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
    While(std::shared_ptr<Expr> condition,std::shared_ptr<Stmt> body);
    std::any accept(StmtVisitor& visitor) override; 
  };

}	
