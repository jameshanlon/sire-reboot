#ifndef PARSER_H
#define PARSER_H

#include "Lex.h"
#include "Tree.h"

#include <list>

#define SYN Syn::get()

class Syn {
public:
  static Syn instance;
  static Syn &get() { return instance; }
  Syn() {};
  ~Syn() {};
  void init() {};
  Tree *formTree();

private:
  Lex::Token curTok;
  void getNextToken();
  void checkFor(Lex::Token);
  void error(const char *);
  
  Tree   *readProg();
  Spec   *readSpec();
  Abbr   *readValAbbr();
  Spec   *readDeclAbbr();
  Spef   *readSpef(bool);
  Def    *readDef();
  Def    *readProc();
  Def    *readServ();
  Def    *readFunc();
  Fml    *readFml();
  Decl   *readIntf();
  Cmd    *readCmd();
  Choice *readChoice();
  Altn   *readAltn();
  Range  *readRange();
  Elem   *readElem();
  Expr   *readExpr();
  Name   *readName();
  
  std::list<Expr*>   *readDims();
  std::list<Name*>   *readNames();
  std::list<Fml*>    *readFmls();
  std::list<Decl*>   *readIntfs();
  std::list<Expr*>   *readActuals();
  std::list<Range*>  *readRep();
  std::list<Choice*> *readChoices();
  std::list<Altn*>   *readAltns();
  template<typename T> std::list<T*> *readList(
        Lex::Token, Lex::Token, Lex::Token, T *(Syn::*)());
};

#endif
