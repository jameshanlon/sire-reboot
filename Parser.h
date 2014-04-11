#ifndef PARSER_H
#define PARSER_H

#include "Parser.h"
#include "Lexer.h"

#define SYN Parser::get()

class Parser {
public:
  static Parser instance;
  static Parser &get() { return instance; }
  
  Lexer::Token curTok;
  
  Parser() {};
  ~Parser() {};
  void init() {};
  //Tree parse();

private:
  void getNextToken();
};

#endif
