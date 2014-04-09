#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"

class Parser {

// The parser

public:
  Lexer *lex;
  Lexer::Token curTok;
  
  Parser(Lexer &lex) : lex(lex) {};
  //Tree parse();

private:
  Lexer::Token getNextToken();
};

Parser::Parser &parser() { return Parser::get(); }


#endif
