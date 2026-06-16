#pragma once
#include "visitor.hpp"
#include "token.hpp"
#include "expr.hpp"
#include <memory>
#include <vector>

namespace Statement {

struct Expression : Stmt {
    std::shared_ptr<Expr> expression;
    Expression(std::shared_ptr<Expr> e) : expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitExpressionStmt(this); }
};

struct Proclaim : Stmt {
    std::shared_ptr<Expr> expression;
    Proclaim(std::shared_ptr<Expr> e) : expression(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitProclaimStmt(this); }
};

struct Assign : Stmt {
    Token name; std::shared_ptr<Expr> value;
    Assign(Token n, std::shared_ptr<Expr> val) : name(std::move(n)), value(std::move(val)) {}
    void accept(StmtVisitor& v) override { v.visitAssignStmt(this); }
};

struct Var : Stmt {
    Token name; std::shared_ptr<Expr> init;
    Var(Token n, std::shared_ptr<Expr> i) : name(std::move(n)), init(std::move(i)) {}
    void accept(StmtVisitor& v) override { v.visitVarStmt(this); }
};

struct Block : Stmt {
    std::vector<std::shared_ptr<Stmt>> statements;
    Block(std::vector<std::shared_ptr<Stmt>> s) : statements(std::move(s)) {}
    void accept(StmtVisitor& v) override { v.visitBlockStmt(this); }
};

struct If : Stmt {
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;
    If(std::shared_ptr<Expr> c, std::shared_ptr<Stmt> t, std::shared_ptr<Stmt> e)
        : condition(std::move(c)), thenBranch(std::move(t)), elseBranch(std::move(e)) {}
    void accept(StmtVisitor& v) override { v.visitIfStmt(this); }
};

} // namespace Statement
