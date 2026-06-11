#include "scanner.hpp"
#include "debug.hpp"
#include "token.hpp"
#include "bignum.hpp"
#include <string>
#include <vector>

Scanner::Scanner(const std::string& source): source(source){}

std::vector<Token> Scanner::scanTokens(){
  while (!isAtEnd()){
     start = current;
     scanToken();
  }
  tokens.emplace_back(TokenType::MY_EOF,"",nullptr,line);
  return tokens;
}

bool Scanner::isAtEnd(){
  return current >= static_cast<int>(source.length());
}

void Scanner::addToken(TokenType type){
  addToken(type,nullptr);
}

void Scanner::addToken(TokenType type, std::any literal){
  std::string text{source.substr(start,current - start)};
  tokens.emplace_back(type,text,literal,line);
}

void Scanner::identifier(){
  while(isAlphaNumeric(peek())) advance();
  std::string text{source.substr(start,current - start)};
  auto it = keywords.find(text);
  TokenType type = it == keywords.end() ? TokenType::IDENTIFIER : it->second;
  addToken(type);
}

void Scanner::number(){
  bool isFloat = false;
  while(isDigit(peek())) advance();
  if(peek() == '.' && isDigit(peekNext())){
    isFloat = true;
    advance();
    while(isDigit(peek())) advance();
  }
  std::string text = source.substr(start, current - start);
  try {
    if(isFloat){
      addToken(TokenType::FLOAT, BigFloat::fromString(text));
    } else {
      addToken(TokenType::INT, BigInt(text));
    }
  } catch(const std::exception& e){
    Debug::error(line, std::string("Erro no literal numérico: ") + e.what());
  }
}

void Scanner::string(){
  while(peek() != '"' && !isAtEnd()){
    if(peek() == '\n'){line++;}
    advance();
  }
  if(isAtEnd()){
    Debug::error(line,"string não foi terminada");
    return;
  }
  advance();
  std::string value = source.substr(start + 1, current - start - 2);
  addToken(TokenType::STRING, value);
}

bool Scanner::match(char expected){
  if(isAtEnd() || source.at(current) != expected) return false;
  current++;
  return true;
}

char Scanner::peek(){
  if (isAtEnd()) return '\0';
  return source.at(current);
}

char Scanner::peekNext(){
  if(current + 1 > static_cast<int>(source.length())) return '\0';
  return source.at(current + 1);
}

bool Scanner::isAlpha(char c){
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Scanner::isAlphaNumeric(char c){
  return isAlpha(c) || isDigit(c);
}

bool Scanner::isDigit(char c){
  return c >= '0' && c <= '9';
}

char Scanner::advance(){
  return source[current++];
}

void Scanner::scanToken(){
  char c = advance();
  switch(c){
  case '(': addToken(TokenType::LEFT_PAREN); break;
  case ')': addToken(TokenType::RIGHT_PAREN); break;
  case '{': addToken(TokenType::LEFT_BRACE); break;
  case '}': addToken(TokenType::RIGHT_BRACE); break;
  case ',': addToken(TokenType::COMMA); break;
  case '.': addToken(TokenType::DOT); break;
  case ';': addToken(TokenType::SEMICOLON); break;
  case '*': addToken(match('=') ? TokenType::STAR_EQUAL  : TokenType::STAR);  break;
  case '[': addToken(TokenType::LEFT_BRACKET); break;
  case ']': addToken(TokenType::RIGHT_BRACKET); break;
  case '!': addToken(match('=') ? TokenType::BANG_EQUAL    : TokenType::BANG);    break;
  case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL   : TokenType::EQUAL);   break;
  case '<': addToken(match('=') ? TokenType::LESS_EQUAL    : TokenType::LESS);    break;
  case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

  case '+':
    if(match('+'))      addToken(TokenType::PLUS_PLUS);
    else if(match('=')) addToken(TokenType::PLUS_EQUAL);
    else                addToken(TokenType::PLUS);
    break;

  case '-':
    if(match('-'))      addToken(TokenType::MINUS_MINUS);
    else if(match('=')) addToken(TokenType::MINUS_EQUAL);
    else                addToken(TokenType::MINUS);
    break;

  case '/':
    if(match('/')){
      while(peek() != '\n' && !isAtEnd()) advance();
    } else if(match('=')){
      addToken(TokenType::SLASH_EQUAL);
    } else {
      addToken(TokenType::SLASH);
    }
    break;

  case ' ': case '\r': case '\t': break;
  case '\n': line++; break;
  case '"': string(); break;
  default:
    if(isDigit(c))      number();
    else if(isAlpha(c)) identifier();
    else Debug::error(line,"caractere nao esperado.");
    break;
  }
}
