#include "expr.hpp"
#include "visitor.hpp"
#include <any>
#include <memory>
#include <utility>

Binary::Binary(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right)
    : left{std::move(left)}, oper{std::move(oper)}, right{std::move(right)} {}
std::any Binary::accept(ExprVisitor &visitor){ return visitor.visitBinaryExpr(shared_from_this()); }

Grouping::Grouping(std::shared_ptr<Expr> expression) : expression{std::move(expression)} {}
std::any Grouping::accept(ExprVisitor &visitor){ return visitor.visitGroupingExpr(shared_from_this()); }

Literal::Literal(std::any value) : value{std::move(value)} {}
std::any Literal::accept(ExprVisitor &visitor){ return visitor.visitLiteralExpr(shared_from_this()); }

Unary::Unary(Token oper, std::shared_ptr<Expr> right)
    : oper{std::move(oper)}, right{std::move(right)} {}
std::any Unary::accept(ExprVisitor &visitor){ return visitor.visitUnaryExpr(shared_from_this()); }

Variable::Variable(Token name) : name{std::move(name)} {}
std::any Variable::accept(ExprVisitor &visitor){ return visitor.visitVariable(shared_from_this()); }

Assign::Assign(Token name, std::shared_ptr<Expr> value)
    : name{std::move(name)}, value{std::move(value)} {}
std::any Assign::accept(ExprVisitor &visitor){ return visitor.visitAssignExpr(shared_from_this()); }

Logical::Logical(std::shared_ptr<Expr> left, Token oper, std::shared_ptr<Expr> right)
    : left{std::move(left)}, oper{std::move(oper)}, right{std::move(right)} {}
std::any Logical::accept(ExprVisitor &visitor){ return visitor.visitLogicalExpr(shared_from_this()); }

TypeOf::TypeOf(std::shared_ptr<Expr> operand) : operand{std::move(operand)} {}
std::any TypeOf::accept(ExprVisitor &visitor){ return visitor.visitTypeOfExpr(shared_from_this()); }

CompoundAssign::CompoundAssign(Token name, Token oper, std::shared_ptr<Expr> value)
    : name{std::move(name)}, oper{std::move(oper)}, value{std::move(value)} {}
std::any CompoundAssign::accept(ExprVisitor &visitor){ return visitor.visitCompoundAssign(shared_from_this()); }

PreIncDec::PreIncDec(Token oper, Token name)
    : oper{std::move(oper)}, name{std::move(name)} {}
std::any PreIncDec::accept(ExprVisitor &visitor){ return visitor.visitPreIncDec(shared_from_this()); }

PostIncDec::PostIncDec(Token name, Token oper)
    : name{std::move(name)}, oper{std::move(oper)} {}
std::any PostIncDec::accept(ExprVisitor &visitor){ return visitor.visitPostIncDec(shared_from_this()); }

StringInterp::StringInterp(std::vector<std::shared_ptr<Expr>> parts)
    : parts{std::move(parts)} {}
std::any StringInterp::accept(ExprVisitor &visitor){ return visitor.visitStringInterp(shared_from_this()); }
