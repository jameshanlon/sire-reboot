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
  void checkFor(Lexer::Token, const char*);
  Tree *readProg();
  Spec *readSpec();
  Decl *readDecl();
  Decl *readAbbr();
  Def *readProc();
  Def *readServ();
  Def *readFunc();
  void readFmls(std::vector<Fml*>&);
  void readInterfaces(std::vector<Decl*>&);
  Fml *readFml();
  Decl *readInterface();
  Cmd *readCmd();
  Name *readName();
  void readNames(std::vector<Name*>&);
};

#endif
