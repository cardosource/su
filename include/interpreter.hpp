#pragma once
#include "token.hpp"
#include "expr.hpp"
#include "visitor.hpp"
#include "stmt.hpp"
#include "environment.hpp"
#include "gc.hpp"
#include "debug.hpp"
#include "bindings.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Interpreter : public ExprVisitor, public Statement::StmtVisitor {
public:
    Interpreter();
    ~Interpreter();

    void interpret(std::vector<std::shared_ptr<Statement::Stmt>>& statements);
    void execute(Statement::Stmt* statement);
    void executeBlock(std::vector<std::shared_ptr<Statement::Stmt>>& stmts, Env* env);

    // ExprVisitor
    SuValue visitBinaryExpr(Binary* expr) override;
    SuValue visitGroupingExpr(Grouping* expr) override;
    SuValue visitLiteralExpr(Literal* expr) override;
    SuValue visitUnaryExpr(Unary* expr) override;
    SuValue visitVariable(Variable* expr) override;
    SuValue visitAssignExpr(Assign* expr) override;
    SuValue visitLogicalExpr(Logical* expr) override;
    SuValue visitTypeOfExpr(TypeOf* expr) override;
    SuValue visitIdOfExpr(IdOf* expr) override;
    SuValue visitCompoundAssign(CompoundAssign* expr) override;
    SuValue visitPreIncDec(PreIncDec* expr) override;
    SuValue visitPostIncDec(PostIncDec* expr) override;
    SuValue visitStringInterp(StringInterp* expr) override;
    SuValue visitListLiteral(ListLiteral* expr) override;
    SuValue visitListIndex(ListIndex* expr) override;   
    SuValue visitLenExpr(Len* expr) override;


    // StmtVisitor
    void visitExpressionStmt(Statement::Expression* stmt) override;
    void visitProclaimStmt(Statement::Proclaim* stmt) override;
    void visitAssignStmt(Statement::Assign* stmt) override;
    void visitVarStmt(Statement::Var* stmt) override;
    void visitBlockStmt(Statement::Block* stmt) override;
    void visitIfStmt(Statement::If* stmt) override;

    Env* global = nullptr;
    static const char* typeStr(const SuValue& v);
    std::string stringify(const SuValue& v);
    
    static Interpreter* instance;

private:
    Env* curr_env = nullptr;

    SuValue evaluate(Expr* expr);
    bool    isTruthy(const SuValue& v);
    bool    isEqual(const SuValue& a, const SuValue& b);
    void    checkNumber(const Token& op, const SuValue& v);
    void    checkNumbers(const Token& op, const SuValue& a, const SuValue& b);
    SuValue applyArith(const Token& op, SuValue a, SuValue b);
    void    gcRegisterEnv(Env* env);
    
    mutable std::unordered_map<uint64_t, uint64_t> inlineIds_;
    mutable uint64_t nextInlineId_ = 1;
    
    uint64_t getInlineId(const SuValue& v) const;
};