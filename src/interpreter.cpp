#include "interpreter.hpp"
#include "debug.hpp"
#include "runtimeError.hpp"
#include "stmt.hpp"
#include "token.hpp"
#include "visitor.hpp"
#include "bignum.hpp"
#include <any>
#include <cmath>
#include <exception>
#include <memory>
#include <string>
#include <iostream>
#include <vector>

Interpreter::Interpreter() {
    global = std::make_shared<Env>();
    curr_env = global;
}

static bool isBigInt  (const std::any& v){ return v.type() == typeid(BigInt);   }
static bool isBigFloat(const std::any& v){ return v.type() == typeid(BigFloat); }
static bool isNum     (const std::any& v){ return isBigInt(v) || isBigFloat(v); }

static BigFloat toBigFloat(const std::any& v){
    if(isBigInt(v)) return BigFloat(std::any_cast<BigInt>(v));
    return std::any_cast<BigFloat>(v);
}

static BigInt one(){ return BigInt(1LL); }

 
std::any Interpreter::applyArith(const Token& oper, std::any left, std::any right){
    checkNumberOperands(oper, left, right);
    bool bothInt = isBigInt(left) && isBigInt(right);
    switch(oper.type){
        case TokenType::PLUS:
        case TokenType::PLUS_EQUAL:
            if(bothInt) return std::any_cast<BigInt>(left) + std::any_cast<BigInt>(right);
            return toBigFloat(left) + toBigFloat(right);
        case TokenType::MINUS:
        case TokenType::MINUS_EQUAL:
            if(bothInt) return std::any_cast<BigInt>(left) - std::any_cast<BigInt>(right);
            return toBigFloat(left) - toBigFloat(right);
        case TokenType::STAR:
        case TokenType::STAR_EQUAL:
            if(bothInt) return std::any_cast<BigInt>(left) * std::any_cast<BigInt>(right);
            return toBigFloat(left) * toBigFloat(right);
        case TokenType::SLASH:
        case TokenType::SLASH_EQUAL:{
            if(isBigInt(right) && std::any_cast<BigInt>(right).isZero())
                throw RuntimeError{oper, "Divisão por zero."};
            if(isBigFloat(right) && toBigFloat(right).mantissa.isZero())
                throw RuntimeError{oper, "Divisão por zero."};
            if(bothInt){
                auto [q, rem] = BigInt::divmod(std::any_cast<BigInt>(left), std::any_cast<BigInt>(right));
                if(rem.isZero()) return q;
                return toBigFloat(left) / toBigFloat(right);
            }
            return toBigFloat(left) / toBigFloat(right);
        }
        default:
            throw RuntimeError{oper, "Operador aritmético desconhecido."};
    }
}

std::any Interpreter::visitLiteralExpr(std::shared_ptr<Literal> expr){ return expr->value; }

std::any Interpreter::visitUnaryExpr(std::shared_ptr<Unary> expr){
    std::any right = evaluate(expr->right);
    switch(expr->oper.type){
        case TokenType::BANG:   return !isTruthy(right);
        case TokenType::MINUS:
            checkNumberOperand(expr->oper, right);
            if(isBigInt(right))   return -std::any_cast<BigInt>(right);
            if(isBigFloat(right)) return -std::any_cast<BigFloat>(right);
            return {};
        default: return {};
    }
}

 
std::any Interpreter::visitPreIncDec(std::shared_ptr<PreIncDec> expr){
    std::any val = curr_env->get(expr->name);
    checkNumberOperand(expr->oper, val);
    std::any newVal;
    if(expr->oper.type == TokenType::PLUS_PLUS)
        newVal = isBigInt(val) ? std::any(std::any_cast<BigInt>(val) + one())
                               : std::any(toBigFloat(val) + BigFloat(one()));
    else
        newVal = isBigInt(val) ? std::any(std::any_cast<BigInt>(val) - one())
                               : std::any(toBigFloat(val) - BigFloat(one()));
    curr_env->assign(expr->name, newVal);
    return newVal;
}

 
std::any Interpreter::visitPostIncDec(std::shared_ptr<PostIncDec> expr){
    std::any val = curr_env->get(expr->name);
    checkNumberOperand(expr->oper, val);
    std::any newVal;
    if(expr->oper.type == TokenType::PLUS_PLUS)
        newVal = isBigInt(val) ? std::any(std::any_cast<BigInt>(val) + one())
                               : std::any(toBigFloat(val) + BigFloat(one()));
    else
        newVal = isBigInt(val) ? std::any(std::any_cast<BigInt>(val) - one())
                               : std::any(toBigFloat(val) - BigFloat(one()));
    curr_env->assign(expr->name, newVal);
    return val; // retorna valor ANTES do incremento
}

 
std::any Interpreter::visitCompoundAssign(std::shared_ptr<CompoundAssign> expr){
    std::any current = curr_env->get(expr->name);
    std::any right   = evaluate(expr->value);
    std::any result  = applyArith(expr->oper, current, right);
    curr_env->assign(expr->name, result);
    return result;
}

bool Interpreter::isTruthy(const std::any& object){
    if(object.type() == typeid(nullptr)) return false;
    if(object.type() == typeid(bool)) return std::any_cast<bool>(object);
    return true;
}

void Interpreter::checkNumberOperand(const Token& oper, const std::any& operand){
    if(isNum(operand)) return;
    throw RuntimeError{oper, "Operand deve ser um número."};
}

void Interpreter::checkNumberOperands(const Token& oper, const std::any& left, const std::any& right){
    if(isNum(left) && isNum(right)) return;
    throw RuntimeError{oper, "Operands devem ser números."};
}

bool Interpreter::isEqual(const std::any& a, const std::any& b){
    if(a.type() == typeid(nullptr) && b.type() == typeid(nullptr)) return true;
    if(a.type() == typeid(nullptr) || b.type() == typeid(nullptr)) return false;
    if(isNum(a) && isNum(b)) return toBigFloat(a) == toBigFloat(b);
    if(a.type() == typeid(std::string) && b.type() == typeid(std::string))
        return std::any_cast<std::string>(a) == std::any_cast<std::string>(b);
    if(a.type() == typeid(bool) && b.type() == typeid(bool))
        return std::any_cast<bool>(a) == std::any_cast<bool>(b);
    return false;
}

std::string Interpreter::stringify(const std::any& object){
    if(object.type() == typeid(nullptr))     return "nil";
    if(object.type() == typeid(bool))        return std::any_cast<bool>(object) ? "true" : "false";
    if(object.type() == typeid(std::string)) return std::any_cast<std::string>(object);
    if(isBigInt(object))                     return std::any_cast<BigInt>(object).toString();
    if(isBigFloat(object))                   return std::any_cast<BigFloat>(object).toString();
    return "stringify: tipo não reconhecido";
}

std::any Interpreter::visitGroupingExpr(std::shared_ptr<Grouping> expr){ return evaluate(expr->expression); }
std::any Interpreter::evaluate(std::shared_ptr<Expr> expr){ return expr->accept(*this); }

std::any Interpreter::visitBinaryExpr(std::shared_ptr<Binary> expr){
    std::any left  = evaluate(expr->left);
    std::any right = evaluate(expr->right);
    switch(expr->oper.type){
        case TokenType::PLUS:
            if(left.type() == typeid(std::string) || right.type() == typeid(std::string))
                return stringify(left) + stringify(right);
            return applyArith(expr->oper, left, right);
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
            return applyArith(expr->oper, left, right);
        case TokenType::GREATER:
            checkNumberOperands(expr->oper, left, right);
            return toBigFloat(left) > toBigFloat(right);
        case TokenType::GREATER_EQUAL:
            checkNumberOperands(expr->oper, left, right);
            return toBigFloat(left) >= toBigFloat(right);
        case TokenType::LESS:
            checkNumberOperands(expr->oper, left, right);
            return toBigFloat(left) < toBigFloat(right);
        case TokenType::LESS_EQUAL:
            checkNumberOperands(expr->oper, left, right);
            return toBigFloat(left) <= toBigFloat(right);
        case TokenType::BANG_EQUAL:  return !isEqual(left, right);
        case TokenType::EQUAL_EQUAL: return isEqual(left, right);
        default: return {};
    }
}

void Interpreter::interpret(std::vector<std::shared_ptr<Statement::Stmt>> &statements){
    for(auto& statement : statements){
        try{ execute(statement); }
        catch(const RuntimeError& e){ Debug::runtimeError(e); }
    }
}

void Interpreter::execute(std::shared_ptr<Statement::Stmt> statement){ statement->accept(*this); }

std::any Interpreter::visitExpressionStmt(std::shared_ptr<Statement::Expression> stmt){
    evaluate(stmt->expression); return {};
}

std::any Interpreter::visitProclaimStmt(std::shared_ptr<Statement::Proclaim> stmt){
    std::cout << stringify(evaluate(stmt->expression)) << '\n';
    return {};
}

std::any Interpreter::visitVariable(std::shared_ptr<Variable> expr){ return curr_env->get(expr->name); }

std::any Interpreter::visitVarStmt(std::shared_ptr<Statement::Var> stmt){
    std::any value = nullptr;
    if(stmt->init != nullptr) value = evaluate(stmt->init);
    curr_env->define(stmt->name.lexeme, std::move(value));
    return {};
}

std::any Interpreter::visitAssignStmt(std::shared_ptr<Statement::Assign> stmt){
    std::any value = evaluate(stmt->value);
    try { curr_env->assign(stmt->name, value); }
    catch(const RuntimeError&){ curr_env->define(stmt->name.lexeme, std::move(value)); }
    return {};
}

std::any Interpreter::visitAssignExpr(std::shared_ptr<Assign> expr){
    std::any value = evaluate(expr->value);
    curr_env->assign(expr->name, value);
    return value;
}

void Interpreter::executeBlock(const std::vector<std::shared_ptr<Statement::Stmt>> &statements,
                               std::shared_ptr<Env> new_env){
    std::shared_ptr<Env> previous = curr_env;
    try{
        curr_env = new_env;
        for(const auto& s : statements) execute(s);
    }catch(...){ curr_env = previous; throw; }
    curr_env = previous;
}

std::any Interpreter::visitBlockStmt(std::shared_ptr<Statement::Block> stmt){
    executeBlock(stmt->statements, std::make_shared<Env>(curr_env));
    return {};
}

std::any Interpreter::visitIfStmt(std::shared_ptr<Statement::If> stmt){
    if(isTruthy(evaluate(stmt->condition))) execute(stmt->thenBranch);
    else if(stmt->elseBranch != nullptr)    execute(stmt->elseBranch);
    return {};
}

std::any Interpreter::visitWhileStmt(std::shared_ptr<Statement::While> stmt){
    while(isTruthy(evaluate(stmt->condition))) execute(stmt->body);
    return {};
}

std::any Interpreter::visitLogicalExpr(std::shared_ptr<Logical> expr){
    std::any left = evaluate(expr->left);
    if(expr->oper.type == TokenType::OR){ if(isTruthy(left)) return left; }
    else                                { if(!isTruthy(left)) return left; }
    return evaluate(expr->right);
}

std::any Interpreter::visitTypeOfExpr(std::shared_ptr<TypeOf> expr){
    std::any value = evaluate(expr->operand);
    if(value.type() == typeid(nullptr))     return std::string("nil");
    if(value.type() == typeid(bool))        return std::string("bool");
    if(value.type() == typeid(std::string)) return std::string("str");
    if(isBigInt(value))                     return std::string("int");
    if(isBigFloat(value))                   return std::string("float");
    return std::string("unknown");
}

 
std::any Interpreter::visitStringInterp(std::shared_ptr<StringInterp> expr){
    std::string result;
    for(auto& part : expr->parts){
        result += stringify(evaluate(part));
    }
    return result;
}
