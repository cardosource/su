#pragma once
// ═══════════════════════════════════════════════════════════════════════════
//  expr.hpp — nós da AST
//  AST gerenciada por shared_ptr (lifetime  definido, sem ciclos)
//  Valores em runtime gerenciados pelo GC (SuValue)
// ═══════════════════════════════════════════════════════════════════════════
#include <memory>
#include <vector>
#include "visitor.hpp"
#include "token.hpp"
#include "gc_bigint.hpp"  

struct Binary : Expr {
    std::shared_ptr<Expr> left;
    Token oper;
    std::shared_ptr<Expr> right;
    Binary(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r);
    SuValue accept(ExprVisitor& v) override;
};

struct Grouping : Expr {
    std::shared_ptr<Expr> expression;
    Grouping(std::shared_ptr<Expr> e);
    SuValue accept(ExprVisitor& v) override;
};

struct Literal : Expr {
    SuValue value;
    Literal(SuValue val);
    SuValue accept(ExprVisitor& v) override;
};

struct Unary : Expr {
    Token oper;
    std::shared_ptr<Expr> right;
    Unary(Token op, std::shared_ptr<Expr> r);
    SuValue accept(ExprVisitor& v) override;
};

struct Variable : Expr {
    Token name;
    Variable(Token n);
    SuValue accept(ExprVisitor& v) override;
};

struct Assign : Expr {
    Token name;
    std::shared_ptr<Expr> value;
    Assign(Token n, std::shared_ptr<Expr> val);
    SuValue accept(ExprVisitor& v) override;
};

struct Logical : Expr {
    std::shared_ptr<Expr> left;
    Token oper;
    std::shared_ptr<Expr> right;
    Logical(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r);
    SuValue accept(ExprVisitor& v) override;
};

struct TypeOf : Expr {
    std::shared_ptr<Expr> operand;
    TypeOf(std::shared_ptr<Expr> e);
    SuValue accept(ExprVisitor& v) override;
};

struct IdOf : Expr {
    Token name;
    IdOf(Token n);
    SuValue accept(ExprVisitor& v) override;
};

struct CompoundAssign : Expr {
    Token name;
    Token oper;
    std::shared_ptr<Expr> value;
    CompoundAssign(Token n, Token op, std::shared_ptr<Expr> val);
    SuValue accept(ExprVisitor& v) override;
};

struct PreIncDec : Expr {
    Token oper;
    Token name;
    PreIncDec(Token op, Token n);
    SuValue accept(ExprVisitor& v) override;
};

struct PostIncDec : Expr {
    Token name;
    Token oper;
    PostIncDec(Token n, Token op);
    SuValue accept(ExprVisitor& v) override;
};

struct StringInterp : Expr {
    std::vector<std::shared_ptr<Expr>> parts;
    StringInterp(std::vector<std::shared_ptr<Expr>> p);
    SuValue accept(ExprVisitor& v) override;
};