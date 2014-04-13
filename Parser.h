#ifndef PARSER_H
#define PARSER_H

#include "Parser.h"
#include "Lexer.h"
#include "Tree.h"

#define SYN Parser::get()

class Parser {
public:
  static Parser instance;
  static Parser &get() { return instance; }
  Parser() {};
  ~Parser() {};
  void init() {};
  Tree *formTree();

private:
  Lexer::Token curTok;
  void getNextToken();
  void checkFor(Lexer::Token t, const char *msg);
  Tree *readProgram(); 
};

#endif
