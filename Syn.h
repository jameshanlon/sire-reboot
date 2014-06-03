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
  void checkFor(Lex::Token, const char*);
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
  Choice *readChoice();
  Altn *readAltn();
  Rep  *readRep();
  IndexRange *readIndexRange();
  Elem *readElem();
  Expr *readExpr();
  Name *readName();
  std::list<Name*> *readNames();

  template<typename T>
    std::list<T*> *readList(
        Lex::Token, Lex::Token, Lex::Token, 
        T *(Syn::*)());
};

#endif
