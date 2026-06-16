#include "astPrinter.hpp"

static std::string toString(Expr* expr) {
    if (!expr) return "nil";
    AstPrinter printer;
    return printer.print(expr);
}

static std::string toString(const std::string& str) {
    return str;
}

static std::string toString(const Token& token) {
    return token.lexeme;
}

static std::string toString(const std::shared_ptr<Expr>& expr) {
    return toString(expr.get());
}

std::string AstPrinter::print(Expr* expr) {
    if (!expr) return "nil";
    SuValue result = expr->accept(*this);
    if (result.isString()) {
        return std::string(result.asString()->data, result.asString()->len);
    }
    return "unknown";
}

std::string AstPrinter::parenthesize(std::string_view name) {
    std::ostringstream buffer;
    buffer << "(" << name << ")";
    return buffer.str();
}

template<typename T>
std::string AstPrinter::parenthesize(std::string_view name, T&& first) {
    std::ostringstream buffer;
    buffer << "(" << name;
    buffer << " " << toString(std::forward<T>(first));
    buffer << ")";
    return buffer.str();
}

template<typename T, typename... Rest>
std::string AstPrinter::parenthesize(std::string_view name, T&& first, Rest&&... rest) {
    std::ostringstream buffer;
    buffer << "(" << name;
    buffer << " " << toString(std::forward<T>(first));
    ((buffer << " " << toString(std::forward<Rest>(rest))), ...);
    buffer << ")";
    return buffer.str();
}

SuValue AstPrinter::visitBinaryExpr(Binary* expr) {
    std::string result = parenthesize(expr->oper.lexeme, expr->left, expr->right);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitGroupingExpr(Grouping* expr) {
    std::string result = parenthesize("group", expr->expression);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitLiteralExpr(Literal* expr) {
    std::string result;
    if (expr->value.isNil()) {
        result = "nil";
    } else if (expr->value.isBool()) {
        result = expr->value.asBool() ? "true" : "false";
    } else if (expr->value.isInt()) {
        result = std::to_string(expr->value.asInt());
    } else if (expr->value.isString()) {
        result = std::string(expr->value.asString()->data, expr->value.asString()->len);
    } else {
        result = "literal";
    }
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitUnaryExpr(Unary* expr) {
    std::string result = parenthesize(expr->oper.lexeme, expr->right);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitVariable(Variable* expr) {
    std::string result = parenthesize(expr->name.lexeme);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitAssignExpr(Assign* expr) {
    std::string result = parenthesize("assign", expr->name.lexeme, expr->value);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitLogicalExpr(Logical* expr) {
    std::string result = parenthesize(expr->oper.lexeme, expr->left, expr->right);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitTypeOfExpr(TypeOf* expr) {
    std::string result = parenthesize("typeof", expr->operand);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitIdOfExpr(IdOf* expr) {
    std::string result = parenthesize("idof", expr->name);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitCompoundAssign(CompoundAssign* expr) {
    std::string result = parenthesize(expr->oper.lexeme, expr->name.lexeme, expr->value);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitPreIncDec(PreIncDec* expr) {
    std::string result = parenthesize(expr->oper.lexeme, expr->name);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitPostIncDec(PostIncDec* expr) {
    std::string result = parenthesize(expr->name.lexeme, expr->oper.lexeme);
    return SuValue::make_obj(SuString::create(result.c_str()));
}

SuValue AstPrinter::visitStringInterp(StringInterp* expr) {
    std::string result = "(";
    for (auto& part : expr->parts) {
        result += " " + toString(part);
    }
    result += " )";
    return SuValue::make_obj(SuString::create(result.c_str()));
}

// Instanciação explícita dos templates
template std::string AstPrinter::parenthesize<std::shared_ptr<Expr>&>(std::string_view, std::shared_ptr<Expr>&);
template std::string AstPrinter::parenthesize<const std::string&, std::shared_ptr<Expr>&>(std::string_view, const std::string&, std::shared_ptr<Expr>&);
template std::string AstPrinter::parenthesize<std::shared_ptr<Expr>&, std::shared_ptr<Expr>&>(std::string_view, std::shared_ptr<Expr>&, std::shared_ptr<Expr>&);
template std::string AstPrinter::parenthesize<Token&>(std::string_view, Token&);
