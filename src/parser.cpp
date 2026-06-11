#include "parser.hpp"
#include "debug.hpp"
#include "expr.hpp"
#include "stmt.hpp"
#include "token.hpp"
#include "visitor.hpp"
#include "bignum.hpp"
#include <exception>
#include <memory>
#include <string>
#include <vector>

#define assert(E)

Parser::Parser(const std::vector<Token>& tokens): tokens(tokens) {}

std::vector<std::shared_ptr<Statement::Stmt>> Parser::parser(){
  std::vector<std::shared_ptr<Statement::Stmt>> statements;
  try{
    while(!isAtEnd()){
      std::shared_ptr<Statement::Stmt> decl = declaration();
      if(decl != nullptr) statements.push_back(decl);
    }
  }catch(const std::exception& e){
    std::cerr << "Exception parser: " << e.what() << "\n";
  }
  return statements;
}

std::shared_ptr<Expr> Parser::expression(){
  return assignment();
}

std::shared_ptr<Expr> Parser::equality(){
  std::shared_ptr<Expr> expr = comparison();
  while(match(TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL)){
    Token oper = previous();
    std::shared_ptr<Expr> right = comparison();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

std::shared_ptr<Expr> Parser::comparison(){
  std::shared_ptr<Expr> expr = term();
  while(match(TokenType::GREATER, TokenType::GREATER_EQUAL,
              TokenType::LESS, TokenType::LESS_EQUAL)){
    Token oper = previous();
    std::shared_ptr<Expr> right = term();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

std::shared_ptr<Expr> Parser::term(){
  std::shared_ptr<Expr> expr = factor();
  while(match(TokenType::MINUS, TokenType::PLUS)){
    Token oper = previous();
    std::shared_ptr<Expr> right = factor();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

std::shared_ptr<Expr> Parser::factor(){
  std::shared_ptr<Expr> expr = unary();
  while(match(TokenType::SLASH, TokenType::STAR)){
    Token oper = previous();
    std::shared_ptr<Expr> right = unary();
    expr = std::make_shared<Binary>(expr, oper, right);
  }
  return expr;
}

std::shared_ptr<Expr> Parser::unary(){
  if(match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS)){
    Token oper = previous();
    Token name = consume(TokenType::IDENTIFIER, "Expected variable name after '" + oper.lexeme + "'.");
    return std::make_shared<PreIncDec>(oper, name);
  }
  if(match(TokenType::BANG, TokenType::MINUS)){
    Token oper = previous();
    std::shared_ptr<Expr> right = unary();
    return std::make_shared<Unary>(oper, right);
  }
  return postfix();
}

std::shared_ptr<Expr> Parser::postfix(){
  std::shared_ptr<Expr> expr = primary();
  if(match(TokenType::PLUS_PLUS, TokenType::MINUS_MINUS)){
    Token oper = previous();
    if(Variable* v = dynamic_cast<Variable*>(expr.get())){
      return std::make_shared<PostIncDec>(v->name, oper);
    }
    throw error(oper, "Expected variable before '" + oper.lexeme + "'.");
  }
  return expr;
}

std::shared_ptr<Expr> Parser::primary(){
  if(match(TokenType::MY_FALSE)) return std::make_shared<Literal>(false);
  if(match(TokenType::MY_TRUE))  return std::make_shared<Literal>(true);
  if(match(TokenType::NIL))      return std::make_shared<Literal>(nullptr);

  if(match(TokenType::TYPEOF)){
    consume(TokenType::LEFT_PAREN, "Expected '(' after 'typeOf'.");
    std::shared_ptr<Expr> operand = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after typeOf operand.");
    return std::make_shared<TypeOf>(operand);
  }

  if(match(TokenType::IDENTIFIER)) return std::make_shared<Variable>(previous());

  if(match(TokenType::INT, TokenType::FLOAT)){
    return std::make_shared<Literal>(previous().literal);
  }

  if(match(TokenType::STRING)){
    Token tok = previous();
    std::string raw = std::any_cast<std::string>(tok.literal);
    // se contém '#' → interpola
    if(raw.find('#') != std::string::npos){
      return parseStringInterp(raw, tok.line);
    }
    return std::make_shared<Literal>(tok.literal);
  }

  if(match(TokenType::LEFT_PAREN)){
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expected ')' after expression.");
    return std::make_shared<Grouping>(expr);
  }

  throw error(peek(), "Expected expression.");
}

template<class...T>
bool Parser::match(T...types){
  assert((... && std::is_same_v<T, TokenType>));
  if((... || check(types))){
    advance();
    return true;
  }
  return false;
}

Token Parser::consume(const TokenType& token, const std::string& message){
  if(check(token)) return advance();
  throw error(peek(), message);
}

bool Parser::check(const TokenType& type){
  if(isAtEnd()) return false;
  return peek().type == type;
}

bool Parser::isAtEnd(){
  return peek().type == TokenType::MY_EOF;
}

Token Parser::advance(){
  if(!isAtEnd()) current++;
  return previous();
}

Token Parser::peek(){
  return tokens.at(current);
}

Token Parser::previous(){
  if(current == 0) return Token{TokenType::MY_EOF, "", nullptr, 0};
  return tokens.at(current - 1);
}

Parser::ParseError Parser::error(const Token& token, const std::string& message){
  Debug::error(token, message);
  return ParseError("");
}

void Parser::synchronize(){
  advance();
  while(!isAtEnd()){
    if(previous().type == TokenType::SEMICOLON) return;
    switch(peek().type){
      case TokenType::CLASS: case TokenType::FUN: case TokenType::VAR:
      case TokenType::FOR:   case TokenType::IF:  case TokenType::WHILE:
      case TokenType::PROCLAIM: case TokenType::RETURN:
        return;
      default: break;
    }
    advance();
  }
}

std::shared_ptr<Statement::Stmt> Parser::statement(){
  if(check(TokenType::IDENTIFIER) && peek().lexeme == "proclaim"){
    advance();
    return proclaimStatement();
  }
  if(match(TokenType::PROCLAIM))   return proclaimStatement();
  if(match(TokenType::IF))         return IfStatement();
  if(match(TokenType::WHILE))      return whileStatement();
  if(match(TokenType::FOR))        return forStatement();
  if(match(TokenType::LEFT_BRACE)) return std::make_shared<Statement::Block>(block());

  if(check(TokenType::IDENTIFIER)){
    size_t next = current + 1;
    if(next < tokens.size()){
      TokenType nt = tokens[next].type;
      if(nt == TokenType::EQUAL)        return assignmentStatement();
      if(nt == TokenType::PLUS_EQUAL  ||
         nt == TokenType::MINUS_EQUAL ||
         nt == TokenType::STAR_EQUAL  ||
         nt == TokenType::SLASH_EQUAL)  return compoundAssignStatement();

    }
  }

  return expressionStatement();
}

std::shared_ptr<Statement::Stmt> Parser::proclaimStatement(){
  std::shared_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Esperado ';' após valor.");
  return std::make_shared<Statement::Proclaim>(value);
}

std::shared_ptr<Statement::Stmt> Parser::expressionStatement(){
  Token exprStart = peek();
  std::shared_ptr<Expr> expr = expression();
  if(!check(TokenType::SEMICOLON)){
    throw error(exprStart, "Esperado ';' após expressão.");
  }
  advance();
  return std::make_shared<Statement::Expression>(expr);
}

std::shared_ptr<Statement::Stmt> Parser::assignmentStatement(){
  Token name = consume(TokenType::IDENTIFIER, "Esperado nome da variável.");
  consume(TokenType::EQUAL, "Esperado '=' após nome da variável.");
  std::shared_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Esperado ';' após valor.");
  return std::make_shared<Statement::Assign>(name, value);
}

std::shared_ptr<Statement::Stmt> Parser::compoundAssignStatement(){
  Token name = consume(TokenType::IDENTIFIER, "Esperado nome da variável.");
  Token oper = advance(); // consome +=, -=, *=, /=
  std::shared_ptr<Expr> value = expression();
  consume(TokenType::SEMICOLON, "Esperado ';' após valor.");
  auto expr = std::make_shared<CompoundAssign>(name, oper, value);
  return std::make_shared<Statement::Expression>(expr);
}

std::shared_ptr<Statement::Stmt> Parser::declaration(){
  try{
    return statement();
  }catch(const std::exception& e){
    std::cerr << "Parse error: " << e.what() << std::endl;
    synchronize();
    return nullptr;
  }
}

template bool Parser::match<TokenType>(TokenType);
template bool Parser::match<TokenType, TokenType>(TokenType, TokenType);
template bool Parser::match<TokenType, TokenType, TokenType>(TokenType, TokenType, TokenType);
template bool Parser::match<TokenType, TokenType, TokenType, TokenType>(TokenType, TokenType, TokenType, TokenType);

std::shared_ptr<Expr> Parser::assignment(){
  std::shared_ptr<Expr> expr = logicalOr();
  if(match(TokenType::EQUAL)){
    Token equals = previous();
    std::shared_ptr<Expr> value = assignment();
    if(Variable *e = dynamic_cast<Variable*>(expr.get())){
      return std::make_shared<Assign>(e->name, value);
    }
    error(std::move(equals), "Invalid assignment target.");
  }
  return expr;
}

std::vector<std::shared_ptr<Statement::Stmt>> Parser::block(){
  std::vector<std::shared_ptr<Statement::Stmt>> statements;
  while(!check(TokenType::RIGHT_BRACE) && !isAtEnd()){
    statements.push_back(declaration());
  }
  consume(TokenType::RIGHT_BRACE, "Expected '}' after block.");
  return statements;
}

std::shared_ptr<Statement::Stmt> Parser::IfStatement(){
  consume(TokenType::LEFT_PAREN, "Expected '(' after 'if'.");
  std::shared_ptr<Expr> condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expected ')' after if condition.");
  std::shared_ptr<Statement::Stmt> thenBranch = statement();
  std::shared_ptr<Statement::Stmt> elseBranch = nullptr;
  if(match(TokenType::ELSE)) elseBranch = statement();
  return std::make_shared<Statement::If>(condition, thenBranch, elseBranch);
}

std::shared_ptr<Expr> Parser::logicalOr(){
  std::shared_ptr<Expr> expr = logicalAnd();
  while(match(TokenType::OR)){
    Token oper = previous();
    std::shared_ptr<Expr> right = logicalAnd();
    expr = std::make_shared<Logical>(expr, std::move(oper), right);
  }
  return expr;
}

std::shared_ptr<Expr> Parser::logicalAnd(){
  std::shared_ptr<Expr> expr = equality();
  while(match(TokenType::AND)){
    Token oper = previous();
    std::shared_ptr<Expr> right = equality();
    expr = std::make_shared<Logical>(expr, std::move(oper), right);
  }
  return expr;
}

std::shared_ptr<Statement::Stmt> Parser::whileStatement(){
  consume(TokenType::LEFT_PAREN, "Expected '(' after 'while'.");
  std::shared_ptr<Expr> condition = expression();
  consume(TokenType::RIGHT_PAREN, "Expected ')' after while condition.");
  std::shared_ptr<Statement::Stmt> body = statement();
  return std::make_shared<Statement::While>(condition, body);
}

std::shared_ptr<Statement::Stmt> Parser::forStatement(){
  consume(TokenType::LEFT_PAREN, "Expected '(' after 'for'.");
  bool isRange = false;
  for(size_t k = current; k < tokens.size(); k++){
    if(tokens[k].type == TokenType::SEMICOLON)   break;
    if(tokens[k].type == TokenType::RIGHT_PAREN) break;
    if(tokens[k].type == TokenType::TO){ isRange = true; break; }
  }

  if(isRange){
    std::string varName = "_for_i";
    std::shared_ptr<Expr> startExpr = std::make_shared<Literal>(std::any(BigInt(0LL)));

    if(check(TokenType::IDENTIFIER)){
      size_t next = current + 1;
      if(next < tokens.size() && tokens[next].type == TokenType::EQUAL){
        Token nameT = advance();       
        varName = nameT.lexeme;
        advance();                     
        startExpr = expression();
      } else if(next < tokens.size() && tokens[next].type == TokenType::TO){
        Token nameT = advance();       
        varName = nameT.lexeme;
      } else {
        startExpr = expression();
      }
    } else {
      startExpr = expression();
    }

    consume(TokenType::TO, "Expected 'to' in range for.");
    std::shared_ptr<Expr> limitExpr = expression();
    std::shared_ptr<Expr> stepExpr = std::make_shared<Literal>(std::any(BigInt(1LL)));
    if(match(TokenType::COMMA)){
      stepExpr = expression();
    }

    consume(TokenType::RIGHT_PAREN, "Expected ')' after for range.");
    std::shared_ptr<Statement::Stmt> body = statement();

    Token varTok{TokenType::IDENTIFIER, varName, std::any{}, 0};

    auto initStmt = std::make_shared<Statement::Assign>(varTok, startExpr);

    auto varExpr  = std::make_shared<Variable>(varTok);
    Token leOp{TokenType::LESS_EQUAL, "<=", std::any{}, 0};
    auto condition = std::make_shared<Binary>(varExpr, leOp, limitExpr);

    Token plusEqOp{TokenType::PLUS_EQUAL, "+=", std::any{}, 0};
    auto incrExpr = std::make_shared<CompoundAssign>(varTok, plusEqOp, stepExpr);
    auto incrStmt = std::make_shared<Statement::Expression>(incrExpr);

    std::vector<std::shared_ptr<Statement::Stmt>> blockStmts;
    blockStmts.push_back(body);
    blockStmts.push_back(incrStmt);
    auto fullBody = std::make_shared<Statement::Block>(blockStmts);
    auto whileStmt = std::make_shared<Statement::While>(condition, fullBody);

    std::vector<std::shared_ptr<Statement::Stmt>> outerBlock;
    outerBlock.push_back(initStmt);
    outerBlock.push_back(whileStmt);
    return std::make_shared<Statement::Block>(outerBlock);
  }


  std::shared_ptr<Statement::Stmt> init = nullptr;
  if(match(TokenType::SEMICOLON)){
    init = nullptr;
  } else {
    if(check(TokenType::IDENTIFIER)){
      size_t next = current + 1;
      if(next < tokens.size()){
        TokenType nt = tokens[next].type;
        if(nt == TokenType::EQUAL){
          Token name = advance();
          advance();
          std::shared_ptr<Expr> val = expression();
          consume(TokenType::SEMICOLON, "Expected ';' after for init.");
          init = std::make_shared<Statement::Assign>(name, val);
        } else if(nt == TokenType::PLUS_EQUAL || nt == TokenType::MINUS_EQUAL ||
                  nt == TokenType::STAR_EQUAL  || nt == TokenType::SLASH_EQUAL){
          Token name = advance();
          Token oper = advance();
          std::shared_ptr<Expr> val = expression();
          consume(TokenType::SEMICOLON, "Expected ';' after for init.");
          auto ca = std::make_shared<CompoundAssign>(name, oper, val);
          init = std::make_shared<Statement::Expression>(ca);
        } else {
          init = expressionStatement();
        }
      } else {
        init = expressionStatement();
      }
    } else {
      init = expressionStatement();
    }
  }

  std::shared_ptr<Expr> condition = nullptr;
  if(!check(TokenType::SEMICOLON)) condition = expression();
  consume(TokenType::SEMICOLON, "Expected ';' after for condition.");

  std::shared_ptr<Expr> increment = nullptr;
  if(!check(TokenType::RIGHT_PAREN)) increment = expression();
  consume(TokenType::RIGHT_PAREN, "Expected ')' after for clauses.");

  std::shared_ptr<Statement::Stmt> body = statement();

  if(increment != nullptr){
    body = std::make_shared<Statement::Block>(
      std::vector<std::shared_ptr<Statement::Stmt>>{
        body, std::make_shared<Statement::Expression>(increment)
      }
    );
  }
  if(condition == nullptr) condition = std::make_shared<Literal>(true);
  body = std::make_shared<Statement::While>(condition, body);
  if(init != nullptr){
    body = std::make_shared<Statement::Block>(
      std::vector<std::shared_ptr<Statement::Stmt>>{init, body}
    );
  }
  return body;
}


std::shared_ptr<Expr> Parser::parseStringInterp(const std::string& raw, int line){
  std::vector<std::shared_ptr<Expr>> parts;
  std::string buf;
  size_t i = 0;

  auto flushBuf = [&](){
    if(!buf.empty()){
      parts.push_back(std::make_shared<Literal>(std::any(buf)));
      buf.clear();
    }
  };

  while(i < raw.size()){
    if(raw[i] == '#'){
      flushBuf();
      i++; 

      if(i < raw.size() && raw[i] == '{'){
        i++;
        size_t start = i;
        int depth = 1;
        while(i < raw.size() && depth > 0){
          if(raw[i] == '{') depth++;
          else if(raw[i] == '}') depth--;
          if(depth > 0) i++;
          else break;
        }
        std::string exprSrc = raw.substr(start, i - start);
        i++; 
        Scanner subScanner(exprSrc);
        std::vector<Token> subTokens = subScanner.scanTokens();
        Parser subParser(subTokens);
        auto subExpr = subParser.expression();
        parts.push_back(subExpr);

      } else {
        size_t start = i;
        while(i < raw.size() && (isalnum(raw[i]) || raw[i] == '_')) i++;
        std::string varName = raw.substr(start, i - start);
        if(varName.empty()){
          buf += '#';
        } else {
          Token nameTok{TokenType::IDENTIFIER, varName, std::any{}, line};
          parts.push_back(std::make_shared<Variable>(nameTok));
        }
      }
    } else {
      buf += raw[i++];
    }
  }
  flushBuf();

  if(parts.size() == 1) return parts[0];
  return std::make_shared<StringInterp>(std::move(parts));
}
