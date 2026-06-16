#include "expr.hpp"
Binary::Binary(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r)
    : left(std::move(l)), oper(std::move(op)), right(std::move(r)) {}

SuValue Binary::accept(ExprVisitor& v) {
    return v.visitBinaryExpr(this);
}

Grouping::Grouping(std::shared_ptr<Expr> e)
    : expression(std::move(e)) {}

SuValue Grouping::accept(ExprVisitor& v) {
    return v.visitGroupingExpr(this);
}

Literal::Literal(SuValue val)
    : value(val) {}

SuValue Literal::accept(ExprVisitor& v) {
    return v.visitLiteralExpr(this);
}

Unary::Unary(Token op, std::shared_ptr<Expr> r)
    : oper(std::move(op)), right(std::move(r)) {}

SuValue Unary::accept(ExprVisitor& v) {
    return v.visitUnaryExpr(this);
}

Variable::Variable(Token n)
    : name(std::move(n)) {}

SuValue Variable::accept(ExprVisitor& v) {
    return v.visitVariable(this);
}

Assign::Assign(Token n, std::shared_ptr<Expr> val)
    : name(std::move(n)), value(std::move(val)) {}

SuValue Assign::accept(ExprVisitor& v) {
    return v.visitAssignExpr(this);
}

Logical::Logical(std::shared_ptr<Expr> l, Token op, std::shared_ptr<Expr> r)
    : left(std::move(l)), oper(std::move(op)), right(std::move(r)) {}

SuValue Logical::accept(ExprVisitor& v) {
    return v.visitLogicalExpr(this);
}

TypeOf::TypeOf(std::shared_ptr<Expr> e)
    : operand(std::move(e)) {}

SuValue TypeOf::accept(ExprVisitor& v) {
    return v.visitTypeOfExpr(this);
}

IdOf::IdOf(Token n)
    : name(std::move(n)) {}

SuValue IdOf::accept(ExprVisitor& v) {
    return v.visitIdOfExpr(this);
}

CompoundAssign::CompoundAssign(Token n, Token op, std::shared_ptr<Expr> val)
    : name(std::move(n)), oper(std::move(op)), value(std::move(val)) {}

SuValue CompoundAssign::accept(ExprVisitor& v) {
    return v.visitCompoundAssign(this);
}

PreIncDec::PreIncDec(Token op, Token n)
    : oper(std::move(op)), name(std::move(n)) {}

SuValue PreIncDec::accept(ExprVisitor& v) {
    return v.visitPreIncDec(this);
}

PostIncDec::PostIncDec(Token n, Token op)
    : name(std::move(n)), oper(std::move(op)) {}

SuValue PostIncDec::accept(ExprVisitor& v) {
    return v.visitPostIncDec(this);
}

StringInterp::StringInterp(std::vector<std::shared_ptr<Expr>> p)
    : parts(std::move(p)) {}

SuValue StringInterp::accept(ExprVisitor& v) {
    return v.visitStringInterp(this);
}