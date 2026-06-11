#include "astPrinter.hpp"
#define assert(E) // REMOVER DEPOIS

std::string AstPrinter::print(std::shared_ptr<Expr> expr) {
  try {
    return std::any_cast<std::string>(expr->accept(*this));
  } catch (const std::bad_any_cast &e) {
    std::cout << "AstPrinter::print -> " << e.what() << "\n"; // debug
    return "something";
  }
}

std::any AstPrinter::visitBinaryExpr(std::shared_ptr<Binary> expr) {
  return parenthesize(expr->oper.lexeme, expr->left, expr->right);
}

std::any AstPrinter::visitGroupingExpr(std::shared_ptr<Grouping> expr) {
  return parenthesize("group", expr->expression);
}

std::any AstPrinter::visitLiteralExpr(std::shared_ptr<Literal> expr) {
  auto &type = expr->value.type();


  if (type == typeid(nullptr)) return "nil";
  else if (type == typeid(std::string)) {
    try {
      return std::any_cast<std::string>(expr->value);
    } catch (const std::bad_any_cast &e) {
      std::cout << "Ast::visitLiteralExpr -> " << e.what() << "\n";
    }
  } else if (type == typeid(int)) { // suporte para int
    return std::to_string(std::any_cast<int>(expr->value));
  } else if (type == typeid(double)) {
    return std::to_string(std::any_cast<double>(expr->value));
  } else if (type == typeid(bool)) {
    if (std::any_cast<bool>(expr->value)) {
      std::string result{"true"};
      return result;
    } else {
      std::string result{"false"};
      return result;
    }
  }
  return "Literal type not recognized";
}

std::any AstPrinter::visitUnaryExpr(std::shared_ptr<Unary> expr) {
  return parenthesize(expr->oper.lexeme, expr->right);
}

template<class... E>
std::string AstPrinter::parenthesize(std::string_view name, E... expr) {
  assert((... && std::is_same_v<E, std::shared_ptr<Expr>>));
  std::ostringstream buffer;
  buffer << "(" << name;
  ((buffer << " " << print(expr)), ...);
  buffer << ")";
  return buffer.str();
}

/*int main(){
  auto expression = std::make_shared<Binary>(
      std::make_shared<Unary>(
        Token(TokenType::MINUS, "-", nullptr, 1), 
        std::make_shared<Literal>(123)
      ),
      Token(TokenType::STAR, "*", nullptr, 1),
      std::make_shared<Grouping>(
       std::make_shared<Literal>(45.67)
      )
  );

  AstPrinter printer;
  std::cout << printer.print(expression) << '\n';
  
  
}*/
