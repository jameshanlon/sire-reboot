#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "Tree.h"

#include <list>

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
  void error(const char *);
  Tree *readProg();
  Spec *readSpec();
  Abbr *readValAbbr();
  Spec *readDeclAbbr();
  std::list<Expr*> *readDims();
  Spef *readSpef(bool);
  Def  *readDef();
  Def  *readProc();
  Def  *readServ();
  Def  *readFunc();
  Fml  *readFml();
  Decl *readIntf();
  Cmd  *readCmd();
  Elem *readElem();
  Expr *readExpr();
  Name *readName();
  std::list<Fml*> *readFmls();
  std::list<Expr*> *readActuals();
  std::list<Decl*> *readIntfs();
  std::list<Name*> *readNames();
};

#endif
