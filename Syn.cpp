#include "Syn.h"
#include "Error.h"

#include <stdio.h>
#include <cassert>

Syn Syn::instance;

void Syn::getNextToken() {
  curTok = LEX.readToken();
}

void Syn::checkFor(Lex::Token t) {
  if(curTok != t) {
    char msg[25];
    sprintf(msg, "'%s' expected", LEX.tokStr(t));
    LEX.error(msg);
  }
  LEX.readToken();
}

void Syn::error(const char *msg) {
  LEX.error(msg);
  getNextToken();
}

Tree *Syn::formTree() {
  Tree *t = readProg();
  return t;
}

// prog    = <spec> ":" <prog>
//         | <seq>;
// spec    = <decl>
//         | <abbr>
//         | <def>
//         | <sim-def>
// sim-def = {0 "&" <def> }
Tree *Syn::readProg() {
  Tree *tree = new Tree();
  
  // Read specifications
  while(curTok == Lex::tVAL 
     || curTok == Lex::tVAR 
     || curTok == Lex::tPROC 
     || curTok == Lex::tSERV 
     || curTok == Lex::tFUNC) {
    Spec *spec = readSpec();
    
    // Read a sequential specification
    switch(curTok) {
    default:
      error("bad separator for specification");
      break;
    // End of specification
    case Lex::tCOLON:
      tree->spec.push_back(spec);
      break;
    // Read simultaneous definitions
    case Lex::tAND:
      if (spec->type != Spec::DEF)
        error("cannot make a simultaneous declaration");
      std::list<Def*> *defs = new std::list<Def*>();
      defs->push_back((Def *) spec);
      do {
        getNextToken();
        defs->push_back(readDef());
      } while (curTok == Lex::tAND);
      SimDef *simDef = new SimDef(defs); 
      tree->spec.push_back(simDef);
      break;
    }
  }

  // Read program command sequence
  while (curTok != Lex::tEOF)
    tree->prog.push_back(readCmd());
  
  return tree;
}

// spec = <decl>
//      | <abbr>
//      | <def>
Spec *Syn::readSpec() {
  switch(curTok) {
  default:            assert(0 && "invalid token");
  case Lex::tEOF:  return NULL;
  case Lex::tVAL:  return readDeclAbbr();
  case Lex::tVAR:  return readDeclAbbr();
  case Lex::tPROC: return readProcDef();
  case Lex::tSERV: return readServDef();
  case Lex::tFUNC: return readFuncDef();
  }
}

// def = "process" ...
//     | "server" ...
//     | "function" ...
Def *Syn::readDef() {
  switch(curTok) {
  default:            error("expecting definition");
  case Lex::tPROC: return readProcDef();
  case Lex::tSERV: return readServDef();
  case Lex::tFUNC: return readFuncDef();
  }
}

// decl        = <spec> {1 "," <name> }
//             | <server-decl>
//             | <hiding-decl>
//             | <sim-decl>
// abbr        = <spef> <name> "is" <elem>
//             | "val" <spef> <name> "is" <elem>
//             | "call" <name> "(" {0 "," <fml> } ")" "is" <elem>
// server-decl = <name> "is" "[" <expr> "]" <server>
//             | <name> "is" <rep> <server>
//             | <name> "is" <server>
// hiding-decl = "from" {1 ":" <decl> "inferface" <name>
// sim-decl    = {0 "&" <decl> }
Spec *Syn::readDeclAbbr() {
  // "val" ...
  bool val = false;
  if (curTok == Lex::tVAL) {
    getNextToken();
    val = true;
  }
  // ... <spef> <name> ...
  Spef *spef = readSpef(val);
  Name *name = readName();
  Spec *res;
  switch (curTok) {
  default: assert(0 && "invalid token");
  
  // ...
  case Lex::tCOLON:
    if (val) 
      error("invalid use of val specifier");
    res = new VarDecl(spef, name);
    break;

  // ... "," {1 "," <name> } ...
  case Lex::tCOMMA:
    if (val) 
      error("invalid use of val specifier");
    res = new VarDecl(spef, readNames());
    break;

  // ... "is" <elem> ...
  case Lex::tIS:
    res = new VarAbbr(spef, name, readElem());
    break;
 
  // ... "(" {0 "," <fml> } ")" "is" <elem> ...
  case Lex::tLPAREN:
    std::list<Fml*> args = readFmls();
    checkFor(Lex::tIS);
    res = new CallAbbr(spef, name, args, readElem());
    break;
  }

  // ... ":"
  checkFor(Lex::tCOLON);
  return res;
}

// decl = <hiding>
// hiding = "from" "[" {1 ":" <decl> } "]" "interface" <elem>
Hiding *Syn::readHidingDecl() {
  checkFor(Lex::tFROM);
  std::list<Decl*> *decls = readHiddenDecls();
  checkFor(Lex::tINTF);
  return new Hiding(readName(), decls);
}

// decl = <spec> {1 "," <name> }
//      | <hiding-decl>
//      | <server-decl>
//      | <sim-decl>
Decl *Syn::readDecl() {
}

// spec = <type>
//      | <type> <name>
//      | <spef> "[" "]"
//      | <spef> "[" <expr> "]"
// type = "var"
//      | "chan"
//      | "call"
//      | "interface" "(" {0 "," <decl> } ")"
//      | "process"
//      | "server"
//      | "function"
Spef *Syn::readSpef(bool val) {
  Lex::Token t = curTok;
  getNextToken();
  switch (t) {
  default: 
    error("invalid specifier");
    return NULL;
 
  // <type> [.][.]...[.]
  case Lex::tVAR:
    return new Spef(Spef::VAR, val, readDims());

  case Lex::tCHAN:
    return new Spef(Spef::CHAN, val, readDims());

  case Lex::tCALL:
    return new Spef(Spef::CALL, val, readDims());

  case Lex::tINTF:
    return new IntfSpef(Spef::INTF, readIntfs(), readDims());

  // <type> <name> [.][.]...[.]
  case Lex::tPROC:
    return new NamedSpef(Spef::PROC, readName(), readDims());
  
  case Lex::tSERV:
    return new NamedSpef(Spef::SERV, readName(), readDims());
  
  case Lex::tFUNC:
    return new NamedSpef(Spef::FUNC, readName(), readDims());
  }
}

// def     = "process" <name> "(" {0 "," <fml>} ")" "is" <process>
// process = <interface> "to" <cmd>
//         | <cmd>
Def *Syn::readProcDef() {
  // "process" <name> "(" {0 "," <fml>} ")" "is" ...
  assert(curTok == Lex::tPROC);
  getNextToken();
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  std::list<Decl*> *intfs = NULL;
  // <interface> "to"
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  return new Proc(name, args, intfs, readCmd());
}

// definition   = "server" <name> "(" {0, <fml>} ")" "inherits" <hiding-decl>
//              | "server" <name> "(" {0, <fml>} ")" "is" <server>
// server       = <interface> ":" <server-spec>
// server-sepec = <declaration>
//              | "{" {0 ":" <decl> "}"
// hiding-decl  = "from" "[" {1 "," <decl> } "]" "interface" <name>
Def *Syn::readServDef() {
  // "server" <name> "(" {0, <fml>} ")" ...
  assert(curTok == Lex::tSERV);
  getNextToken();
  Name *name = readName();
  std::list<Fml*> *args = readFmls();

  // ... "inherits" <hiding-decl>
  if(curTok == Lex::tINHRT) {
    getNextToken();
    return new InhrtServ(name, args, readHidingDecl());
  }

  // ... "is" <cmd>
  checkFor(Lex::tIS);
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  return new Serv(name, args, intfs, NULL);
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Syn::readFuncDef() {
  assert(curTok == Lex::tFUNC);
  getNextToken();
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  return new Func(name, args, readExpr());
}

// <decl>
// decl = "call" {1 "," <name> "(" <fml> ")" }
Decl *Syn::readIntf() {
  switch(curTok) {
    default: 
      error("invalid interface");
      return NULL;

    // Chanend interface
    case Lex::tCHAN: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      if (curTok != Lex::tCOMMA)
        return new VarDecl(spef, name);
      // Multiple names
      else {
        getNextToken();
        std::list<Name*> *names = readNames();
        names->insert(names->begin(), name);
        return new VarDecl(spef, names);
      }
    }
    
    // Call interface
    case Lex::tCALL: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      std::list<Fml*> *args = readFmls();
      if (curTok != Lex::tCOMMA)
        return new CallDecl(spef, name, args);
      // Multiple calls
      else {
        std::list<Name*> *names = new std::list<Name*>();
        std::list<std::list<Fml*>*> *argss = new std::list<std::list<Fml*>*>(); 
        names->insert(names->begin(), name);
        while (curTok == Lex::tCOMMA) {
          getNextToken();
          names->push_back(readName());
          argss->push_back(readFmls());
        }
        return new CallDecl(spef, names, argss);
      }
    }
  }
}

// fml = <spef> {1 "," <name> }
//     | "val" <spef> {1 "," <name> }
Fml *Syn::readFml() {
  bool val = false;
  if (curTok == Lex::tVAL) {
    val = true;
    getNextToken();
  }
  switch(curTok) {
  default: 
      error("invalid argument");
      return NULL;
  
  // "val" ...
  // "interface" ...
  // "process" ...
  // "server" ...
  // "function" ...
  case Lex::tVAR:
  case Lex::tINTF:
  case Lex::tPROC:
  case Lex::tSERV:
  case Lex::tFUNC: {
      Spef *spef = readSpef(val);
      Name *name = readName();
      if (curTok != Lex::tCOMMA) 
        return new Fml(spef, name);
      // Multiple names
      else {
        std::list<Name*> *names = readNames();
        names->insert(names->begin(), name);
        return new Fml(spef, names);
      }
    }
  }
}

// cmd        = <prim-cmd> 
//            | <struct-cmd>
//            | <instance>
//            | <call>
//            | "seq" <rep> ":" <cmd>
//            | <spec> ":" <cmd>
// prim-cmd   = <ass>
//            | <connect> 
//            | <in>
//            | <out>
//            | <skip>
//            | <stop>
// struct-cmd = "{" <par>" "}"
//            | "{" <seq> "}"
//            | <alt>
//            | <case>
//            | <cond>
//            | <loop>
Cmd *Syn::readCmd() {
  switch(curTok) {
  default: 
    error("bad command");
    return NULL;
  
  // "skip"
  case Lex::tSKIP:
    getNextToken();
    return new Skip();
  
  // "stop"
  case Lex::tSTOP:
    getNextToken();
    return new Stop();

  // "connect" <elem> "to" <elem>
  case Lex::tCONNECT: {
    getNextToken();
    Elem *source = readElem();
    checkFor(Lex::tTO);
    Elem *target = readElem();
    return new Connect(source, target);
  }

  // ass      = <name> ":=" <expr>
  // in       = <elem> "?" <elem>
  // out      = <elem> "!" <elem>
  // instance = <name> "(" {0 "," <expr> } ")"
  // call     = <name> "." <name> "(" {0 "," <expr> } ")"
  case Lex::tNAME: {
    Name *name = readName();
    getNextToken();
    switch (curTok) {
    default:
      error("expecting assignment, input, output, instance or call");
      return NULL;
    
    // ":=" <expr>
    case Lex::tASS:
      getNextToken();
      return new Ass(name, readExpr());
    
    // "?" <elem>
    case Lex::tIN:
      getNextToken();
      return new In(name, readElem());
    
    // "!" <expr>
    case Lex::tOUT:
      getNextToken();
      return new Out(name, readExpr());

    // ... "(" {0 "," <expr> } ")"
    case Lex::tLPAREN: {
      getNextToken();
      std::list<Expr*> *actuals = readActuals();
      checkFor(Lex::tRPAREN);
      return new Instance(name, actuals);
    }
    
    // ... "." <name> "(" {0 "," <expr> } ")"
    case Lex::tDOT: {
        getNextToken();
        Name *field = readName();
        checkFor(Lex::tLPAREN);
        std::list<Expr*> *actuals = readActuals();
        checkFor(Lex::tRPAREN);
        return new Call(name, field, actuals);
      }
    }
  } 

  // cond = "if" <expr> "do" <cmd>
  //      | "if" <expr> "then" <cmd> "else" <cmd>
  case Lex::tIF: {
      getNextToken();
      Expr* expr = readExpr();
      switch(curTok) {
        default:
          error("expecting 'do' or 'then'");
          return NULL;

        case Lex::tDO:
          return new IfD(expr, readCmd());
        
        case Lex::tTHEN: {
          Cmd *thenCmd = readCmd();
          checkFor(Lex::tELSE);
          return new IfTE(expr, thenCmd, readCmd());
      }
      return NULL;
    }
  }

  // cond = "test" "{" {0 "|" <choice> } "}"
  //      | "test" <rep> <choice>
  case Lex::tTEST:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;
  
    case Lex::tLPAREN:
      return new Test(readChoices());
    
    case Lex::tLSQ:
      return new RepTest(readRep(), readChoice());
    }

  // alt = "alt" "{" {0 "|" <altn> } "}"
  //     | "alt" <rep> <altn>
  case Lex::tALT:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;

    case Lex::tLPAREN:
      return new Alt(readAltns());
    
    case Lex::tLSQ:
      return new RepAlt(readRep(), readAltn());
    }

  // case = "case" <expr> "{" {0 "|" <selection> } "}"
  //      | "case" <expr> <rep> <selection>
  case Lex::tCASE: {
    getNextToken();
    Expr *expr = readExpr();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;
    
    case Lex::tLCURLY:
      return new Case(expr, readSelects());
    
    case Lex::tLSQ:
      return new RepCase(expr, readRep(), readSelect());
    }
  }

  // loop = "while" <expr> "do" <cmd>
  case Lex::tWHILE: {
    getNextToken();
    Expr *expr = readExpr();
    checkFor(Lex::tDO);
    return new While(expr, readCmd());
  }

  // loop = "do" <cmd> "while" <expr>
  case Lex::tDO: {
    getNextToken();
    Cmd *cmd = readCmd();
    checkFor(Lex::tWHILE);
    return new Do(cmd, readExpr());
  }

  // loop = "until" <expr> "do" <cmd>
  case Lex::tUNTIL: {
    getNextToken();
    Expr *expr = readExpr();
    checkFor(Lex::tDO);
    return new Until(expr, readCmd());
  }

  // <spec> ":" <cmd>
  case Lex::tVAL: 
  case Lex::tVAR:
  case Lex::tCHAN: 
  case Lex::tCALL: 
  case Lex::tINTF: {
    Spec *spec = readSpec();
    checkFor(Lex::tCOLON);
    return new CmdSpec(spec, readCmd());
  }

  // Disallowed
  case Lex::tPROC:
  case Lex::tSERV:
  case Lex::tFUNC:
    error("definition in specification of command");
    readSpec();
    return new CmdSpec(NULL, readCmd());
  }
}

// choice = <cond>
//        | <spec> ":" <choice>
//        | <expr> ":" <cmd>
Choice *Syn::readChoice() {
  switch(curTok) {
  default: break;
  
  // <cond>
  case Lex::tTEST:
    return new NestedChoice((Test*) readCmd());

  // <spec> ":" <choice>
  case Lex::tVAL:
  case Lex::tVAR: 
  case Lex::tCHAN: 
  case Lex::tCALL: 
  case Lex::tINTF: {
    Spec *spec = readSpec();
    checkFor(Lex::tCOLON);
    return new SpecChoice(spec, readChoice());
  }
  
  // Disallowed specifications
  case Lex::tPROC:
  case Lex::tSERV:
  case Lex::tFUNC:
    error("definition in specification of choice");
    readSpec();
    return new SpecChoice(NULL, readChoice());
  }

  // <expr> ":" <cmd>
  Expr *expr = readExpr();
  checkFor(Lex::tCOLON);
  return new GuardedChoice(expr, readCmd());
}

// altn  = <alt>
//       | <spec> ":" <altn>
//       | <guard> ":" <cmd>
// guard = <elem> "?" <elem>
//       | <expr> "&" <elem> "?" <elem>
//       | <expr> "&" <skip>
Altn *Syn::readAltn() {
  switch(curTok) {
  default: break;
  
  // <alt>
  case Lex::tALT:
    return new NestedAltn((Alt*) readCmd());

  // <spec> ":" <altn>
  case Lex::tVAL:
  case Lex::tVAR: 
  case Lex::tCHAN: 
  case Lex::tCALL: 
  case Lex::tINTF: {
    Spec *spec = readSpec();
    checkFor(Lex::tCOLON);
    return new SpecAltn(spec, readAltn());
  }
  
  // Disallowed specifications
  case Lex::tPROC:
  case Lex::tSERV:
  case Lex::tFUNC:
    error("definition in specification of alternative");
    readSpec();
    return new SpecAltn(NULL, readAltn());
  }

  // <elem> "?" <elem> ":" <cmd>
  if (curTok == Lex::tNAME) {
    Elem *dst = readElem();
    checkFor(Lex::tIN);
    Elem *src = readElem();
    checkFor(Lex::tCOLON);
    return new UnguardedAltn(dst, src, readCmd());
  }

  // <expr> "&" ...
  Expr *expr = readExpr();
  checkFor(Lex::tAND);
  // ... <skip> ":" <cmd>
  if (curTok == Lex::tSKIP) {
    getNextToken();
    checkFor(Lex::tCOLON);
    return new SkipAltn(expr, readCmd());
  }
  // ... <elem> "?" <elem> ":" <cmd>
  Elem *dst = readElem();
  checkFor(Lex::tIN);
  Elem *src = readElem();
  checkFor(Lex::tCOLON);
  return new GuardedAltn(expr, dst, src, readCmd());  
}

// select = <expr> ":" <cmd>
//        | "else" <cmd>
Select *Syn::readSelect() {
  // "else" <cmd>
  if (curTok == Lex::tELSE) {
    getNextToken();
    return new ElseSelect(readCmd());
  }
  // <expr> ":" <cmd>
  Expr *expr = readExpr();
  checkFor(Lex::tCOLON);
  return new GuardedSelect(expr, readCmd());
}

// index-range = <name> "=" <expr> "for" <expr>
//             | <name> "=" <expr> "for" <expr> "step" <expr>
Range *Syn::readRange() {
  Name *name = readName();
  checkFor(Lex::tEQ);
  Expr *base = readExpr();
  checkFor(Lex::tFOR);
  Expr *count = readExpr();
  Expr *step = NULL;
  if (curTok == Lex::tSTEP) {
    getNextToken();
    step = readExpr();
  }
  return new Range(name, base, count, step);
}

// "(" {0 "," <fml> } ")"
inline std::list<Fml*> *Syn::readFmls() {
  return readList<Fml>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, 
      &Syn::readFml);
}

// "(" {0 "," <decl> } ")"
inline std::list<Decl*> *Syn::readIntfs() {
  return readList<Decl>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, 
      &Syn::readIntf);
}

// "[" {1 ":" <decl> } "]"
inline std::list<Decl*> *Syn::readHiddenDecls() {
  return readList<Decl>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOLON, 
      &Syn::readDecl);
}

// "(" {0 "," <expr> } ")"
inline std::list<Expr*> *Syn::readActuals() {
  return readList<Expr>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, 
      &Syn::readExpr);
}

// "[" {1 "," <index-range> } "]"
inline std::list<Range*> *Syn::readRep() {
  return readList<Range>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOMMA,
      &Syn::readRange);
}

// "{" {0 "|" <choice> } "}"
inline std::list<Choice*> *Syn::readChoices() {
  return readList<Choice>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, 
      &Syn::readChoice);
}

// "{" {0 "|" <altn> } "}"
inline std::list<Altn*> *Syn::readAltns() {
  return readList<Altn>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, 
      &Syn::readAltn);
}

// "{" {0 "|" <select> } "}"
inline std::list<Select*> *Syn::readSelects() {
  return readList<Select>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, 
      &Syn::readSelect);
}

// Read a list of T
// <left> {0 <sep> <item> } <right>
template<typename T>
std::list<T*> *Syn::readList(
    Lex::Token left, Lex::Token right, Lex::Token sep, 
    T *(Syn::*readItem)()) {
  checkFor(left);
  std::list<T*> *l = new std::list<T*>();
  do {
    l->push_back((this->*readItem)());
    getNextToken();
  } while (curTok != sep);
  checkFor(right);
  return l;
}

// elem = ...
Elem *Syn::readElem() {
  return NULL;
}

// <name>
Name *Syn::readName() {
  std::string *s = &LEX.s;
  checkFor(Lex::tNAME);
  return new Name(*s);
}

// expr = ...
Expr *Syn::readExpr() {
  return NULL;
}

// "[" <expr> "]" "[" <expr> "]" ... "[" <expr> "]"
std::list<Expr*> *Syn::readDims() {
  if (curTok != Lex::tLSQ) 
    return NULL;
  else {
    std::list<Expr*> *lengths = new std::list<Expr*>();
    while (curTok == Lex::tLSQ) {
      lengths->push_back(readExpr());
      checkFor(Lex::tRSQ);
      getNextToken();
    }
    return lengths;
  }
}

// {0 "," <name> }
std::list<Name*> *Syn::readNames() {
  std::list<Name*> *names = new std::list<Name*>();
  do {
    getNextToken();
    // In formal lists, don't read next specifier
    if (curTok != Lex::tNAME)
      break;
    names->push_back(readName());
  } while (curTok != Lex::tCOMMA);
  return names;
}

