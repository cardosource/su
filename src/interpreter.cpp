#include "interpreter.hpp"
#include "debug.hpp"
#include "runtimeError.hpp"
#include "bignum.hpp"
#include "gc.hpp"
#include <cstring>
#include <cstdio>
#include <sstream>
#include <iostream>
#include <memory>
#include <functional>

Interpreter* Interpreter::instance = nullptr;
Interpreter* g_interpreter = nullptr;


Interpreter::Interpreter(){
    global   = new Env();
    curr_env = global;
    instance = this;
    g_interpreter = this;
}

Interpreter::~Interpreter(){
    delete global;
}

uint64_t Interpreter::getInlineId(const SuValue& v) const {
    uint64_t key = 0;
    if (v.isInt()) {
        key = (1ULL << 60) | (static_cast<uint64_t>(v.asInt()) & 0x0FFFFFFFFFFFFFFFULL);
    } 
    else if (v.isBool()) {
        key = (2ULL << 60) | (v.asBool() ? 1 : 0);
    }
    else if (v.isNil()) {
        key = (3ULL << 60);
    }
    else {
        key = nextInlineId_;
    }
    
    auto it = inlineIds_.find(key);
    if (it != inlineIds_.end()) {
        return it->second;
    }
    
    uint64_t id = nextInlineId_++;
    uint64_t fakeAddr = 0x7F0000000000ULL | id;
    inlineIds_[key] = fakeAddr;
    return fakeAddr;
}

static bool isNumVal(const SuValue& v){
    return v.isInt() ||
           (v.isObj() && (v.obj->type == ObjType::BIGINT ||
                          v.obj->type == ObjType::BIGFLOAT));
}

static double toDouble(const SuValue& v){
    if(v.isInt()) return static_cast<double>(v.asInt());
    if(v.isObj() && v.obj->type == ObjType::BIGFLOAT)
        return reinterpret_cast<SuBigFloat*>(v.obj)->value;
    if(v.isObj() && v.obj->type == ObjType::BIGINT){
        auto* bi = reinterpret_cast<SuBigInt*>(v.obj);
        try { return std::stod(bi->value.toString()); } catch(...){ return 0.0; }
    }
    return 0.0;
}

static SuValue makeBigInt(const BigInt& val){
    void* mem = gcAlloc(sizeof(SuBigInt));
    if(!mem) return SuValue::nil();
    SuBigInt* obj = new(mem) SuBigInt(val);
    obj->type = ObjType::BIGINT;
    return SuValue::make_obj(obj);
}

static SuValue makeBigInt(BigInt&& val){
    void* mem = gcAlloc(sizeof(SuBigInt));
    if(!mem) return SuValue::nil();
    SuBigInt* obj = new(mem) SuBigInt(std::move(val));
    obj->type = ObjType::BIGINT;
    return SuValue::make_obj(obj);
}


const char* Interpreter::typeStr(const SuValue& v){
    if(v.isNil())  return "nil";
    if(v.isBool()) return "bool";
    if(v.isInt())  return "int";
    if(v.isObj()){
        switch(v.obj->type){
            case ObjType::BIGINT:   return "int";
            case ObjType::BIGFLOAT: return "float";
            case ObjType::STRING:   return "str";
            case ObjType::LIST:     return "list";
            default: break;
        }
    }
    return "unknown";
}


std::string Interpreter::stringify(const SuValue& v){
    if(v.isNil())  return "nil";
    if(v.isBool()) return v.asBool() ? "true" : "false";
    if(v.isInt())  return std::to_string(v.asInt());
    if(v.isObj()){
        switch(v.obj->type){
            case ObjType::STRING:
                return std::string(v.asString()->data, v.asString()->len);
            case ObjType::BIGINT:
                return reinterpret_cast<SuBigInt*>(v.obj)->value.toString();
            case ObjType::BIGFLOAT:{
                double d = reinterpret_cast<SuBigFloat*>(v.obj)->value;
                char buf[64];
                snprintf(buf, sizeof(buf), "%g", d);
                return buf;
            }
            case ObjType::LIST:{
                const SuList* list = reinterpret_cast<const SuList*>(v.obj);
                std::string result = "[";
                for (size_t i = 0; i < list->length; i++) {
                    if (i > 0) result += ", ";
                    result += stringify(list->elements[i]);
                }
                result += "]";
                return result;
            }
            default: break;
        }
    }
    return "unknown";
}


void Interpreter::checkNumber(const Token& op, const SuValue& v){
    if(!isNumVal(v)) throw RuntimeError(op, "Operand must be a number.");
}

void Interpreter::checkNumbers(const Token& op, const SuValue& a, const SuValue& b){
    if(!isNumVal(a) || !isNumVal(b))
        throw RuntimeError(op, "Operands must be numbers.");
}

SuValue Interpreter::applyArith(const Token& op, SuValue a, SuValue b){
    checkNumbers(op, a, b);

    if(a.isInt() && b.isInt()){
        int64_t ai = a.asInt(), bi = b.asInt();
        switch(op.type){
            case TokenType::PLUS:
            case TokenType::PLUS_EQUAL:  return SuValue::make_int(ai + bi);
            case TokenType::MINUS:
            case TokenType::MINUS_EQUAL: return SuValue::make_int(ai - bi);
            case TokenType::STAR:
            case TokenType::STAR_EQUAL:  return SuValue::make_int(ai * bi);
            case TokenType::SLASH:
            case TokenType::SLASH_EQUAL:
                if(bi == 0) throw RuntimeError(op, "Division by zero.");
                if(ai % bi == 0) return SuValue::make_int(ai / bi);
                return SuValue::make_obj(SuBigFloat::create(
                    static_cast<double>(ai) / static_cast<double>(bi)));
            default: break;
        }
    }

    bool aBI = a.isObj() && a.obj->type == ObjType::BIGINT;
    bool bBI = b.isObj() && b.obj->type == ObjType::BIGINT;
    if((aBI || a.isInt()) && (bBI || b.isInt())){
        BigInt ai = a.isInt() ? BigInt((long long)a.asInt())
                              : reinterpret_cast<SuBigInt*>(a.obj)->value;
        BigInt bi = b.isInt() ? BigInt((long long)b.asInt())
                              : reinterpret_cast<SuBigInt*>(b.obj)->value;
        switch(op.type){
            case TokenType::PLUS:
            case TokenType::PLUS_EQUAL:  return makeBigInt(ai + bi);
            case TokenType::MINUS:
            case TokenType::MINUS_EQUAL: return makeBigInt(ai - bi);
            case TokenType::STAR:
            case TokenType::STAR_EQUAL:  return makeBigInt(ai * bi);
            case TokenType::SLASH:
            case TokenType::SLASH_EQUAL:{
                if(bi.isZero()) throw RuntimeError(op, "Division by zero.");
                auto [q, rem] = BigInt::divmod(ai, bi);
                if(rem.isZero()) return makeBigInt(q);
                return SuValue::make_obj(SuBigFloat::create(
                    toDouble(a) / toDouble(b)));
            }
            default: break;
        }
    }

    double af = toDouble(a), bf = toDouble(b);
    switch(op.type){
        case TokenType::PLUS:
        case TokenType::PLUS_EQUAL:  return SuValue::make_obj(SuBigFloat::create(af + bf));
        case TokenType::MINUS:
        case TokenType::MINUS_EQUAL: return SuValue::make_obj(SuBigFloat::create(af - bf));
        case TokenType::STAR:
        case TokenType::STAR_EQUAL:  return SuValue::make_obj(SuBigFloat::create(af * bf));
        case TokenType::SLASH:
        case TokenType::SLASH_EQUAL:
            if(bf == 0.0) throw RuntimeError(op, "Division by zero.");
            return SuValue::make_obj(SuBigFloat::create(af / bf));
        default: break;
    }
    throw RuntimeError(op, "Unknown arithmetic operator.");
}

SuValue Interpreter::evaluate(Expr* expr){ return expr->accept(*this); }

bool Interpreter::isTruthy(const SuValue& v){
    if(v.isNil())  return false;
    if(v.isBool()) return v.asBool();
    return true;
}

bool Interpreter::isEqual(const SuValue& a, const SuValue& b){
    if(a.isNil() && b.isNil()) return true;
    if(a.isNil() || b.isNil()) return false;
    if(a.isBool() && b.isBool()) return a.asBool() == b.asBool();
    if(isNumVal(a) && isNumVal(b)) return toDouble(a) == toDouble(b);
    if(a.isString() && b.isString())
        return strcmp(a.asString()->c_str(), b.asString()->c_str()) == 0;
    if(a.isList() && b.isList()) {
        const SuList* la = a.asList();
        const SuList* lb = b.asList();
        if(la->length != lb->length) return false;
        for(size_t i = 0; i < la->length; i++) {
            if(!isEqual(la->elements[i], lb->elements[i])) return false;
        }
        return true;
    }
    return false;
}


SuValue Interpreter::visitLiteralExpr(Literal* expr){ return expr->value; }

SuValue Interpreter::visitGroupingExpr(Grouping* expr){
    return evaluate(expr->expression.get());
}


SuValue Interpreter::visitUnaryExpr(Unary* expr){
    SuValue right = evaluate(expr->right.get());
    switch(expr->oper.type){
        case TokenType::BANG:
            return SuValue::make_bool(!isTruthy(right));
        case TokenType::MINUS:
            checkNumber(expr->oper, right);
            if(right.isInt()) return SuValue::make_int(-right.asInt());
            if(right.isObj() && right.obj->type == ObjType::BIGINT){
                BigInt neg = -reinterpret_cast<SuBigInt*>(right.obj)->value;
                return makeBigInt(std::move(neg));
            }
            if(right.isObj() && right.obj->type == ObjType::BIGFLOAT)
                return SuValue::make_obj(SuBigFloat::create(
                    -reinterpret_cast<SuBigFloat*>(right.obj)->value));
            break;
        default: break;
    }
    return SuValue::nil();
}


SuValue Interpreter::visitBinaryExpr(Binary* expr){
    SuValue left  = evaluate(expr->left.get());
    SuValue right = evaluate(expr->right.get());
    switch(expr->oper.type){
        case TokenType::PLUS:
            if(left.isString() || right.isString()){
                std::string s = stringify(left) + stringify(right);
                return SuValue::make_obj(SuString::create(s.c_str()));
            }
            if(left.isList() && right.isList()) {
                const SuList* la = left.asList();
                const SuList* lb = right.asList();
                SuList* result = la->concat(lb);
                if (!result) {
                    throw RuntimeError(expr->oper, "List concatenation overflow");
                }
                return SuValue::make_obj(result);
            }
            return applyArith(expr->oper, left, right);
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
            return applyArith(expr->oper, left, right);
        case TokenType::GREATER:
            checkNumbers(expr->oper, left, right);
            return SuValue::make_bool(toDouble(left) > toDouble(right));
        case TokenType::GREATER_EQUAL:
            checkNumbers(expr->oper, left, right);
            return SuValue::make_bool(toDouble(left) >= toDouble(right));
        case TokenType::LESS:
            checkNumbers(expr->oper, left, right);
            return SuValue::make_bool(toDouble(left) < toDouble(right));
        case TokenType::LESS_EQUAL:
            checkNumbers(expr->oper, left, right);
            return SuValue::make_bool(toDouble(left) <= toDouble(right));
        case TokenType::BANG_EQUAL:
            return SuValue::make_bool(!isEqual(left, right));
        case TokenType::EQUAL_EQUAL:
            return SuValue::make_bool(isEqual(left, right));
        default:
            return SuValue::nil();
    }
}


SuValue Interpreter::visitLogicalExpr(Logical* expr){
    SuValue left = evaluate(expr->left.get());
    if(expr->oper.type == TokenType::OR){ if(isTruthy(left)) return left; }
    else                                { if(!isTruthy(left)) return left; }
    return evaluate(expr->right.get());
}


SuValue Interpreter::visitVariable(Variable* expr){
    return curr_env->get(expr->name);
}


SuValue Interpreter::visitAssignExpr(Assign* expr){
    SuValue val = evaluate(expr->value.get());
    curr_env->assign(expr->name, val);
    return val;
}

SuValue Interpreter::visitTypeOfExpr(TypeOf* expr){
    SuValue v = evaluate(expr->operand.get());
    return SuValue::make_obj(SuString::create(typeStr(v)));
}

SuValue Interpreter::visitIdOfExpr(IdOf* expr){
    SuValue v = curr_env->get(expr->name);
    char buf[32];
    
    if (v.isObj() && v.obj != nullptr) {
        snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)v.obj);
    } 
    else {
        uint64_t id = getInlineId(v);
        snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)id);
    }
    
    return SuValue::make_obj(SuString::create(buf));
}

SuValue Interpreter::visitLenExpr(Len* expr) {
    SuValue v = evaluate(expr->operand.get());
    
    if (v.isList()) {
        SuList* list = v.asList();
        return SuValue::make_int(list->length);
    }
    
    if (v.isString()) {
        SuString* str = v.asString();
        return SuValue::make_int(str->len);
    }
    
    throw RuntimeError(Token{TokenType::LEN, "len", std::any{}, 0},
                       "len() only works on strings and lists (got: " + std::string(typeStr(v)) + ")");
}

SuValue Interpreter::visitCompoundAssign(CompoundAssign* expr){
    SuValue cur = curr_env->get(expr->name);
    SuValue rhs = evaluate(expr->value.get());
    SuValue res = applyArith(expr->oper, cur, rhs);
    curr_env->assign(expr->name, res);
    return res;
}

SuValue Interpreter::visitPreIncDec(PreIncDec* expr){
    SuValue val = curr_env->get(expr->name);
    checkNumber(expr->oper, val);
    SuValue newVal;
    if(val.isInt()){
        int64_t d = expr->oper.type == TokenType::PLUS_PLUS ? 1 : -1;
        newVal = SuValue::make_int(val.asInt() + d);
    } else {
        Token op{expr->oper.type == TokenType::PLUS_PLUS
                 ? TokenType::PLUS : TokenType::MINUS, "", {}, expr->oper.line};
        newVal = applyArith(op, val, SuValue::make_int(1));
    }
    curr_env->assign(expr->name, newVal);
    return newVal;
}

SuValue Interpreter::visitPostIncDec(PostIncDec* expr){
    SuValue val = curr_env->get(expr->name);
    checkNumber(expr->oper, val);
    SuValue newVal;
    if(val.isInt()){
        int64_t d = expr->oper.type == TokenType::PLUS_PLUS ? 1 : -1;
        newVal = SuValue::make_int(val.asInt() + d);
    } else {
        Token op{expr->oper.type == TokenType::PLUS_PLUS
                 ? TokenType::PLUS : TokenType::MINUS, "", {}, expr->oper.line};
        newVal = applyArith(op, val, SuValue::make_int(1));
    }
    curr_env->assign(expr->name, newVal);
    return val;
}

SuValue Interpreter::visitStringInterp(StringInterp* expr){
    std::string result;
    for(auto& part : expr->parts)
        result += stringify(evaluate(part.get()));
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue Interpreter::visitListLiteral(ListLiteral* expr){
    SuValue elems[SuList::MAX_ELEMENTS];
    size_t count = 0;
    
    for (auto& e : expr->elements) {
        if (count >= SuList::MAX_ELEMENTS) {
            Token tok{TokenType::MY_EOF, "", std::any{}, 0};
            throw RuntimeError(tok, "List too large (max 128 elements)");
        }
        elems[count++] = evaluate(e.get());
    }
    
    SuList* list = SuList::fromArray(elems, count);
    if (!list) {
        Token tok{TokenType::MY_EOF, "", std::any{}, 0};
        throw RuntimeError(tok, "Failed to create list");
    }
    return SuValue::make_obj(list);
}

SuValue Interpreter::visitListIndex(ListIndex* expr) {
    SuValue listVal = evaluate(expr->list.get());
    SuValue indexVal = evaluate(expr->index.get());
    
    if (!listVal.isList()) {
        Token tok{TokenType::LEFT_BRACKET, "", std::any{}, 0};
        throw RuntimeError(tok, "Cannot index non-list value (type: " + std::string(typeStr(listVal)) + ")");
    }
    
    if (!indexVal.isInt()) {
        Token tok{TokenType::LEFT_BRACKET, "", std::any{}, 0};
        throw RuntimeError(tok, "List index must be an integer (got: " + std::string(typeStr(indexVal)) + ")");
    }
    
    int64_t idx = indexVal.asInt();
    SuList* list = listVal.asList();
    
    if (idx < 0 || idx >= static_cast<int64_t>(list->length)) {
        Token tok{TokenType::LEFT_BRACKET, "", std::any{}, 0};
        throw RuntimeError(tok, "List index out of bounds: " + std::to_string(idx) + " (size: " + std::to_string(list->length) + ")");
    }
    
    return list->elements[idx];
}

void Interpreter::visitExpressionStmt(Statement::Expression* stmt){
    evaluate(stmt->expression.get());
}

void Interpreter::visitProclaimStmt(Statement::Proclaim* stmt){
    std::cout << stringify(evaluate(stmt->expression.get())) << '\n';
}

void Interpreter::visitAssignStmt(Statement::Assign* stmt){
    SuValue val = evaluate(stmt->value.get());
    curr_env->define(stmt->name.lexeme, val, stmt->name.line, false);
}

void Interpreter::visitVarStmt(Statement::Var* stmt){
    SuValue val = SuValue::nil();
    if(stmt->init) val = evaluate(stmt->init.get());
    curr_env->define(stmt->name.lexeme, val, stmt->name.line, false);
}

void Interpreter::executeBlock(std::vector<std::shared_ptr<Statement::Stmt>>& stmts,
                               Env* env){
    Env* previous = curr_env;
    curr_env = env;
    try {
        for(auto& s : stmts) execute(s.get());
    } catch(...){
        curr_env = previous;
        delete env;
        throw;
    }
    curr_env = previous;
    delete env;
}

void Interpreter::visitBlockStmt(Statement::Block* stmt){
    Env* blockEnv = new Env(curr_env);
    executeBlock(stmt->statements, blockEnv);
}

void Interpreter::visitIfStmt(Statement::If* stmt){
    if(isTruthy(evaluate(stmt->condition.get()))) execute(stmt->thenBranch.get());
    else if(stmt->elseBranch)                     execute(stmt->elseBranch.get());
}

void Interpreter::execute(Statement::Stmt* stmt){
    stmt->accept(*this);
}

void Interpreter::interpret(std::vector<std::shared_ptr<Statement::Stmt>>& statements){
    for(auto& s : statements){
        try { execute(s.get()); }
        catch(const RuntimeError& e){ Debug::runtimeError(e); }
    }
}

void Interpreter::gcRegisterEnv(Env* env){
    if(env) env->gcMark();
}