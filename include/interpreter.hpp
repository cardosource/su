#pragma once
#include "token.hpp"
#include "expr.hpp"
#include "debug.hpp"
#include "visitor.hpp"
#include <algorithm>
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "stmt.hpp"
#include "environment.hpp"

class Interpreter : public ExprVisitor, public Statement::StmtVisitor {
public:
    Interpreter();
    void interpret(std::vector<std::shared_ptr<Statement::Stmt>> &statements);
    void execute(std::shared_ptr<Statement::Stmt> statement);
    void executeBlock(const std::vector<std::shared_ptr<Statement::Stmt>> &statements, std::shared_ptr<Env> new_env);

    std::any visitBinaryExpr(std::shared_ptr<Binary> expr) override;
    std::any visitGroupingExpr(std::shared_ptr<Grouping> expr) override;
    std::any visitLiteralExpr(std::shared_ptr<Literal> expr) override;
    std::any visitUnaryExpr(std::shared_ptr<Unary> expr) override;
    std::any visitVariable(std::shared_ptr<Variable> expr) override;
    std::any visitAssignExpr(std::shared_ptr<Assign> expr) override;
    std::any visitLogicalExpr(std::shared_ptr<Logical> expr) override;
    std::any visitTypeOfExpr(std::shared_ptr<TypeOf> expr) override;
    std::any visitCompoundAssign(std::shared_ptr<CompoundAssign> expr) override;
    std::any visitPreIncDec(std::shared_ptr<PreIncDec> expr) override;
    std::any visitStringInterp(std::shared_ptr<StringInterp> expr) override;
    std::any visitPostIncDec(std::shared_ptr<PostIncDec> expr) override;

    std::any visitExpressionStmt(std::shared_ptr<Statement::Expression> stmt) override;
    std::any visitProclaimStmt(std::shared_ptr<Statement::Proclaim> stmt) override;
    std::any visitAssignStmt(std::shared_ptr<Statement::Assign> stmt) override;
    std::any visitVarStmt(std::shared_ptr<Statement::Var> stmt) override;
    std::any visitBlockStmt(std::shared_ptr<Statement::Block> stmt) override;
    std::any visitIfStmt(std::shared_ptr<Statement::If> stmt) override;
    std::any visitWhileStmt(std::shared_ptr<Statement::While> stmt) override;

    std::shared_ptr<Env> global;
    std::string stringify(const std::any& object);

private:
    void checkNumberOperand(const Token& oper, const std::any& operand);
    void checkNumberOperands(const Token& oper, const std::any& left, const std::any& right);
    bool isTruthy(const std::any& object);
    bool isEqual(const std::any& a, const std::any& b);
    std::any evaluate(std::shared_ptr<Expr> expr);
    std::any applyArith(const Token& oper, std::any left, std::any right);

    std::unordered_map<std::shared_ptr<Expr>, int> locals;
    std::shared_ptr<Env> curr_env;
};
