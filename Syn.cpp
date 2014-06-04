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

// program = <spec> ":" <program>
//         | <seq>;
// spec    = <decl>
//         | <abbr>
//         | <def>
// def     = {0 "&" <def>}
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
  case Lex::tVAL:  return readValAbbr();
  case Lex::tVAR:  return readDeclAbbr();
  case Lex::tPROC: return readProc();
  case Lex::tSERV: return readServ();
  case Lex::tFUNC: return readFunc();
  }
}

// def = "process" ...
//     | "server" ...
//     | "function" ...
Def *Syn::readDef() {
  switch(curTok) {
  default:            error("expecting definition");
  case Lex::tPROC: return readProc();
  case Lex::tSERV: return readServ();
  case Lex::tFUNC: return readFunc();
  }
}

// abbr = <spef> <name> "is" <elem>
//      | "call" <name> "(" {0 "," <fml> } ")" "is" <name>
Abbr *Syn::readValAbbr() {
  getNextToken();
  Spef *s = readSpef(true);
  Name *n = readName();
  return new VarAbbr(s, n, readElem()); 
}

// spec = <decl>
//      | <abbr>
// decl = <spec> {1 "," <name> }
// abbr = <spec> <name> "is" <element>
Spec *Syn::readDeclAbbr() {
  Spef *spef = readSpef(false);
  Name *name = readName();
  Spec *res;
  switch (curTok) {
  default: assert(0 && "invalid token");
  
  // Single-name declaration
  case Lex::tCOLON:
    res = new VarDecl(spef, name);
    break;

  // Name list declaration
  case Lex::tCOMMA:
    res = new VarDecl(spef, readNames());
    break;
  
  // Abbreviation
  case Lex::tIS:
    res = new VarAbbr(spef, name, readElem());
    break;
  }

  checkFor(Lex::tCOLON);
  return res;
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

// def = process <name> "(" {0 "," <fml>} ")" "is" <cmd>
Def *Syn::readProc() {
  assert(curTok == Lex::tPROC);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::tLPAREN);
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tRPAREN);
  checkFor(Lex::tIS);
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  return new Proc(name, args, intfs, readCmd());
}

// definition  = "server" <name> "(" {0, <fml>} ")" "inherits" <hiding-decl>
//             | "server" <name> "(" {0, <fml>} ")" "is" <cmd>
// hiding-decl = "from" { {1 : <decl> } } "interface" <name>
Def *Syn::readServ() {
  assert(curTok == Lex::tSERV);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::tLPAREN);
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tRPAREN);

  // Inheriting definition
  if(curTok == Lex::tINHRT) {
    // TODO
    // return ...
  }

  checkFor(Lex::tIS);
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  // read server decls
  return new Serv(name, args, intfs, NULL);
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Syn::readFunc() {
  assert(curTok == Lex::tFUNC);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::tLPAREN);
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tRPAREN);
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
  
  case Lex::tSKIP:
    getNextToken();
    return new Skip();
  
  case Lex::tSTOP:
    getNextToken();
    return new Stop();

  case Lex::tCONNECT: {
    getNextToken();
    Elem *source = readElem();
    checkFor(Lex::tTO);
    Elem *target = readElem();
    return new Connect(source, target);
  }

  // assignment, input, output, instance, call
  case Lex::tNAME: {
    Name *name = readName();
    getNextToken();
    switch (curTok) {
    default:
      error("expecting assignment, input, output, instance or call");
      return NULL;
    
    case Lex::tASS:
      getNextToken();
      return new Ass(name, readExpr());
    
    case Lex::tIN:
      getNextToken();
      return new In(name, readElem());
    
    case Lex::tOUT:
      getNextToken();
      return new Out(name, readExpr());

    // Instance
    case Lex::tLPAREN: {
      getNextToken();
      std::list<Expr*> *actuals = readActuals();
      checkFor(Lex::tRPAREN);
      return new Instance(name, actuals);
    }
    
    // Call
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

  // Structured commands
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
  //      | "alt" <rep> <selection>
  case Lex::tCASE:
    // TODO
    return NULL;

  // loop = "while" <expr> "do" <cmd>
  case Lex::tWHILE:
    // TODO
    return NULL;

  // loop = "do" <cmd> "while" <expr>
  case Lex::tDO:
    // TODO
    return NULL;

  // loop = "until" <expr> "do" <cmd>
  case Lex::tUNTIL:
    // TODO
    return NULL;

  // Declaration or abbreviation
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
  
  // Nested test
  case Lex::tTEST:
    return new NestedChoice((Test*) readCmd());

  // Choice specification
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

  // Choice (assume <expr>)
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
  
  // Nested alt
  case Lex::tALT:
    return new NestedAltn((Alt*) readCmd());

  // Alternative specification
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

  // Alternative
  // TODO: unguarded, guarded, skip
  //Expr *expr = readExpr();
  //checkFor(Lex::tCOLON);
  //return new Altn(expr, elem, elem, readCmd());
  return NULL;
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

inline std::list<Fml*> *Syn::readFmls() {
  return readList<Fml>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, 
      &Syn::readFml);
}

inline std::list<Decl*> *Syn::readIntfs() {
  return readList<Decl>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOMMA, 
      &Syn::readIntf);
}

inline std::list<Expr*> *Syn::readActuals() {
  return readList<Expr>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, 
      &Syn::readExpr);
}

inline std::list<Range*> *Syn::readRep() {
  return readList<Range>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOMMA,
      &Syn::readRange);
}

inline std::list<Choice*> *Syn::readChoices() {
  return readList<Choice>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, 
      &Syn::readChoice);
}

inline std::list<Altn*> *Syn::readAltns() {
  return readList<Altn>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, 
      &Syn::readAltn);
}

// Read a list of T
// <left> {0 <sep> <item> } <right>
// item = Fml | Decl | Expr | Range
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

