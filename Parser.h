#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Tree.h"

#include <vector>

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
  Spec *readSpec();
  Decl *readDecl();
  Decl *readAbbr();
  Def *readFunction();
  Def *readProcess();
  Def *readServer();
  std::vector<Formal*> *readFmlList();
  Cmd *readCmd();
  Name *readName();
  std::vector<Name*> *readNameList();
};

#endif
