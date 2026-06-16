#pragma once
#include "gc.hpp"

struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct Variable;
struct Assign;
struct Logical;
struct TypeOf;
struct IdOf;
struct CompoundAssign;
struct PreIncDec;
struct PostIncDec;
struct StringInterp;

struct ExprVisitor {
    virtual SuValue visitBinaryExpr(Binary* expr) = 0;
    virtual SuValue visitGroupingExpr(Grouping* expr) = 0;
    virtual SuValue visitLiteralExpr(Literal* expr) = 0;
    virtual SuValue visitUnaryExpr(Unary* expr) = 0;
    virtual SuValue visitVariable(Variable* expr) = 0;
    virtual SuValue visitAssignExpr(Assign* expr) = 0;
    virtual SuValue visitLogicalExpr(Logical* expr) = 0;
    virtual SuValue visitTypeOfExpr(TypeOf* expr) = 0;
    virtual SuValue visitIdOfExpr(IdOf* expr) = 0;
    virtual SuValue visitCompoundAssign(CompoundAssign* expr) = 0;
    virtual SuValue visitPreIncDec(PreIncDec* expr) = 0;
    virtual SuValue visitPostIncDec(PostIncDec* expr) = 0;
    virtual SuValue visitStringInterp(StringInterp* expr) = 0;
    virtual ~ExprVisitor() = default;
};

struct Expr {
    virtual SuValue accept(ExprVisitor& v) = 0;
    virtual ~Expr() = default;
};

namespace Statement {
struct Expression;
struct Proclaim;
struct Assign;
struct Var;
struct Block;
struct If;

struct StmtVisitor {
    virtual void visitExpressionStmt(Expression* stmt) = 0;
    virtual void visitProclaimStmt(Proclaim* stmt) = 0;
    virtual void visitAssignStmt(Assign* stmt) = 0;
    virtual void visitVarStmt(Var* stmt) = 0;
    virtual void visitBlockStmt(Block* stmt) = 0;
    virtual void visitIfStmt(If* stmt) = 0;
    virtual ~StmtVisitor() = default;
};

struct Stmt {
    virtual void accept(StmtVisitor& v) = 0;
    virtual ~Stmt() = default;
};
}
