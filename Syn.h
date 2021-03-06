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
  
  Tree       *readProg();
  Spec       *readSpec();
  Spec       *readSpecEnd(Spec*);
  Spec       *readServerSpec();
  Spec       *readProcessSpec();
  Spec       *readFunctionSpec();
  HidingDecl *readHidingDecl();
  Spef       *readSpef(bool);
  Def        *readDef();
  Def        *readProcessDef();
  Def        *readServerDef();
  Def        *readFunctionDef();
  Fml        *readFml();
  Decl       *readIntf();
  Server     *readServer();
  Process    *readProcess();
  Cmd        *readCmd();
  Choice     *readChoice();
  Altn       *readAltn();
  Select     *readSelect();
  Range      *readRange();
  Elem       *readElem();
  Name       *readName();
  Expr       *readExpr();
  Valof      *readValof();
  Operand    *readOperand();
  bool        isOp(Lex::Token);
  
  std::list<Expr*>   *readDims();
  std::list<Name*>   *readNames();
  std::list<Fml*>    *readFmls();
  std::list<Decl*>   *readIntfs();
  std::list<Spec*>   *readSpecs();
  std::list<Spec*>   *readHiddens();
  std::list<Expr*>   *readActuals();
  std::list<Range*>  *readRep();
  std::list<Choice*> *readChoices();
  std::list<Altn*>   *readAltns();
  std::list<Select*> *readSelects();
  std::list<Cmd*>    *readSeq();
  template<typename T> std::list<T*> *readList(
        Lex::Token, Lex::Token, Lex::Token, T *(Syn::*)());
};

#endif
