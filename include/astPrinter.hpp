#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include "expr.hpp"

class AstPrinter : public ExprVisitor {
private:
    std::string parenthesize(std::string_view name);
    
    template<typename T>
    std::string parenthesize(std::string_view name, T&& arg);
    
    template<typename T, typename... Rest>
    std::string parenthesize(std::string_view name, T&& first, Rest&&... rest);
    
public:
    std::string print(Expr* expr);

    SuValue visitBinaryExpr(Binary* expr) override;
    SuValue visitGroupingExpr(Grouping* expr) override;
    SuValue visitLiteralExpr(Literal* expr) override;
    SuValue visitUnaryExpr(Unary* expr) override;
    SuValue visitVariable(Variable* expr) override;
    SuValue visitAssignExpr(Assign* expr) override;
    SuValue visitLogicalExpr(Logical* expr) override;
    SuValue visitTypeOfExpr(TypeOf* expr) override;
    SuValue visitIdOfExpr(IdOf* expr) override;
    SuValue visitLenExpr(Len* expr) override;
    SuValue visitCompoundAssign(CompoundAssign* expr) override;
    SuValue visitPreIncDec(PreIncDec* expr) override;
    SuValue visitPostIncDec(PostIncDec* expr) override;
    SuValue visitStringInterp(StringInterp* expr) override;
    SuValue visitListLiteral(ListLiteral* expr) override;
    SuValue visitListIndex(ListIndex* expr) override;
};
