enum class TokenType {

  // Tokens de Caracteres Únicos
  LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, RIGHT_BRACKET, LEFT_BRACKET,
  COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

  // Tokens de um ou dois Caracteres
  BANG, BANG_EQUAL,
  EQUAL, EQUAL_EQUAL,
  GREATER, GREATER_EQUAL,
  LESS, LESS_EQUAL,

  // Operadores de atribuição composta
  PLUS_EQUAL,   // +=
  MINUS_EQUAL,  // -=
  STAR_EQUAL,   // *=
  SLASH_EQUAL,  // /=

  // Incremento / Decremento
  PLUS_PLUS,    // ++
  MINUS_MINUS,  // --

  // Tokens literais
  IDENTIFIER, STRING,
  INT,    // número sem ponto: 12, 0, 100
  FLOAT,  // número com ponto: 12.00, 3.14

  // Palavras-chave
  AND, CLASS, ELSE, MY_FALSE, FUN, FOR, IF, NIL, OR,
  PROCLAIM, RETURN, SUPER, THIS, MY_TRUE, VAR, WHILE,
  TYPEOF,

  // Token para final de declaração
  MY_EOF

};
