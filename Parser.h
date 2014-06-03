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
  std::list<Fml*> *readFmls();
  Decl *readIntf();
  std::list<Decl*> *readIntfs();
  Cmd  *readCmd();
  std::list<Expr*> *readActuals();
  Choice *readChoice();
  std::list<Choice*> *readChoices();
  Choice *readAltn();
  std::list<Altn*> *readAltns();
  Rep  *readRep();
  Elem *readElem();
  Expr *readExpr();
  Name *readName();
  std::list<Name*> *readNames();
};

#endif
