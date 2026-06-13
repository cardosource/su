#pragma once
#include <iostream>
#include <sstream>
#include "expr.hpp"


class AstPrinter: public ExprVisitor {
  private:
    template<class... E>
      std::string parenthesize(std::string_view name, E... expr);
  public:
    std::string print(std::shared_ptr<Expr> expr);

    std::any visitBinaryExpr(std::shared_ptr<Binary> expr);
    std::any visitGroupingExpr(std::shared_ptr<Grouping> expr);
    std::any visitLiteralExpr(std::shared_ptr<Literal> expr);
    std::any visitUnaryExpr(std::shared_ptr<Unary> expr);
};
