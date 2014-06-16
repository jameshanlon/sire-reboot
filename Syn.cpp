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
    
    // <spec>
    Spec *spec = readSpec();
    tree->spec.push_back(spec);
    
    // ... ":"
    if (curTok == Lex::tCOLON)
      getNextToken();
    else
      break;
  }

  // ... {1 ";" <cmd> }
  while (curTok != Lex::tEOF)
    tree->prog.push_back(readCmd());
  
  return tree;
}

// spef = <type>
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
 
  // <type> {0, "[" <expr> "]" }
  case Lex::tVAR:
    return new Spef(Spef::VAR, val, readDims());

  case Lex::tCHAN:
    return new Spef(Spef::CHAN, val, readDims());

  case Lex::tCALL:
    return new Spef(Spef::CALL, val, readDims());

  case Lex::tINTF:
    return new IntfSpef(Spef::INTF, readIntfs(), readDims());

  // <type> <name> {0 "[" <expr> "]" }
  case Lex::tPROC:
    return new NamedSpef(Spef::PROC, readName(), readDims());
  
  case Lex::tSERV:
    return new NamedSpef(Spef::SERV, readName(), readDims());
  
  case Lex::tFUNC:
    return new NamedSpef(Spef::FUNC, readName(), readDims());
  }
}

// spec        = <decl>
//             | <abbr>
//             | <def>
//             | <sim-spec>
// decl        = <spef> {1 "," <name> }
//             | <server-decl>
//             | <hiding-decl>
// abbr        = <spef> <name> "is" <elem>
//             | "val" <spef> <name> "is" <elem>
// def         = <server-def>
//             | <process-def>
//             | <function-def>
// sim-spec    = {2 "&" <spec> }
Spec *Syn::readSpec() {
  Spec *res;
  switch(curTok) {
  default:        assert(0 && "invalid token");
  case Lex::tEOF: return NULL;
  
  // <decl> | <abbr>
  case Lex::tVAL:  
  case Lex::tVAR: 
  case Lex::tCHAN: 
  case Lex::tCALL: 
  case Lex::tINTF: { 
      // "val" ...
      bool val = false;
      if (curTok == Lex::tVAL) {
        getNextToken();
        val = true;
      }
      // ... <spef> <name> ...
      Spef *spef = readSpef(val);
      Name *name = readName();
      if (val && spef->type != Spef::VAR)
        error("invalid use of 'val' specifier");
      
      switch (curTok) {
      default: assert(0 && "invalid token");
      
      // ...
      case Lex::tCOLON:
        if (val) 
          error("invalid use of 'val' specifier");
        res = new VarDecl(spef, name);
        break;

      // ... "," {1 "," <name> } ...
      case Lex::tCOMMA:
        if (val) 
          error("invalid use of 'val' specifier");
        res = new VarDecl(spef, readNames());
        break;

      // ... "is" <elem> ...
      case Lex::tIS:
        res = new VarAbbr(spef, name, readElem());
        break;
      }
    }

  // ... "from" ... 
  case Lex::tFROM:
    res = readHiding();
    break;
  
  // def  = "server" ... 
  // decl = "server" ...
  // abbr = "server" ...
  case Lex::tSERV:
    res = readServerSpec();
    break;

  // def  = "process" ...
  // abbr = "process" ...
  case Lex::tPROC: 
    res = readProcSpec();
    break;
  
  // def  = "function" ...
  // abbr = "function" ...
  case Lex::tFUNC:
    res = readFuncSpec();
    break;
  }

  // Separator
  switch (curTok) {
  default:
    error("expected ':' or '&'");
    return NULL;

  // ... ":"
  case Lex::tCOLON:
    return res;
  
  // ... "&" {1 "&" <spec> }
  case Lex::tAND: {
      std::list<Spec*> *specs = new std::list<Spec*>();
      specs->push_back(res);
      do {
        getNextToken();
        specs->push_back(readSpec());
      } while (curTok == Lex::tAND);
      return new SimSpec(specs);
    }
  }
}

// TODO: server spef with dimensions
// decl = "server" <name> "is" {0 "[" <expr> "]" } <server>
//      | "server" <name> "is" <rep> <server>
//      | "server" <name> "is" <server>
// abbr = "server" <name> "is" <elem>
// def  = "server" <name> "(" {0 "," <fml> } ")" ...
Spec *Syn::readServerSpec() {
    checkFor(Lex::tSERVER);
    Name *name = readName();
    switch(curTok) {
    default:
      error("expected '(' or 'is'");
      return NULL;
    
    // ... "(" {0 "," <fml> } ")" "is" <server>
    // ... "(" {0 "," <fml> } ")" "inherits" <hiding>
    case Lex::tLPAREN: {
        std::list<Fml*> args = readFmls();
        switch(curTok) {
        default: error("expecting 'is' or 'inherits'"); return NULL;
        
        // ... "is" <server>
        case Lex::tIS:
          return new ServerDecl(name, args, readServer());
        
        // ... "inherits" <hiding-decl>
        case Lex::tINRTS:
          getNextToken();
          return new InhrtServerDef(n, args, readHiding());
        }
      }
    
    // ... <decl> | <abbr>
    case Lex::tIS:
      getNextToken();
      switch(curTok) {
      default:
        error("expected '[', 'interface', 'inherits' or instance");
        return NULL;

      // ... <server>
      case Lex::tINTF:
        return readServer();
     
      // ... <rep> <server>
      case Lex::tLSQ:
        return new RServerDecl(n, readRep(), readServer());
      
      // ... <name> "(" {0 "," <expr> } ")"
      // ... <name>
      case Lex::tNAME: {
          Name *server = readName();
          if (curTok == Lex::tLPAREN) {
            std::list<Expr*> args = readActuals();
            return new ServerDecl(name, server, args);
          }
          else
            return new ServerAbbr(name, readElem());
        }
      }
    }
}

// TODO
// TODO: process spef with dimensions
// def  = "process" <name> "(" {0 "," <fml> } ")" "is" <process>
// abbr = "process" <name> "is" <elem>
Spec *Syn::readProcSpec() {
  checkFor(Lex::tPROC);
  Name *name = readName();
  switch(curTok) {
  default:
    error("expected '(' or 'is'");
    return NULL;
    
    // ... "(" {0 "," <fml> } ")" "is" <process>
    case Lex::tLPAREN: {
        std::list<Fml*> args = readFmls();
        checkFor(Lex::tIS);
        return new ProcDef(name, args, readProc());
      }

    // ... "is" <elem>
    case Lex::tIS: {
        getNextToken();
        Elem *elem = readElem();
        return ProcAbbr(name, elem);
      }
  }
  return NULL
}

// TODO
// TODO: function spef with dimensions
// def  = "function" <name> "(" {0 "," <fml> } ")" "is" <expr>
// abbr = "function" <name> "is" <elem>
Spec *Syn::readFuncSpec() {
  checkFor(Lex::tFUNC);
  return NULL;
}

// decl = <hiding>
// hiding = "from" "[" {1 ":" <decl> } "]" "interface" <elem>
HidingDecl *Syn::readHiding() {
  checkFor(Lex::tFROM);
  std::list<Spec*> *decls = readHiddens();
  checkFor(Lex::tINTF);
  return new HidingDecl(readName(), decls);
}

// def         = "server" <name> "(" {0, <fml> } ")" "is" <server>
//             | "server" <name> "(" {0, <fml> } ")" "inherits" <hiding>
// server      = <interface> "to" <server-spec>
// server-spec = <decl>
//             | "{" {0 ":" <decl> "}"
// hiding      = "from" "[" {1 "," <decl> } "]" "interface" <name>
Def *Syn::readServDef() {
  // "server" <name> "(" {0, <fml>} ")" ...
  checkFor(Lex::tSERVER);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();

  switch(curTok) {
  default: error("expecting 'is or 'inherits'"); return NULL;

  // ... "is" ...
  case Lex::tIS:
    getNextToken();
    return new ServerDef(name, args, readServer());
  
  // ... "inherits" <hiding-decl>
  case Lex::tINHRT:
    getNextToken();
    return new InhrtServDef(name, args, readHiding());
  }
}

// def     = "process" <name> "(" {0 "," <fml> } ")" "is" <process>
Def *Syn::readProcDef() {
  checkFor(Lex::tPROC);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  return new ProcDef(name, args, readProc());
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Syn::readFuncDef() {
  checkFor(Lex::tFUNC);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  return new FuncDef(name, args, readExpr());
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
  case Lex::tSERVER:
  case Lex::tPROC:
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

// server = <server-spec>
//        | <interface> "to" <server-spec>
Server *Syn::readServer() {
  // TODO finish
  std::list<Decl*> *intfs = NULL;
  // ... "interface" "(" {0 "," <decl> } ")" "to" ...
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  // TODO read server body
  return new Server(intfs, NULL);
}

// process = <cmd>
//         | <interface> "to" <cmd>
Process *Syn::readProc() {
  // TODO finish
  std::list<Decl*> *intfs = NULL;
  // <interface> "(" {0 "," <decl> } ")" "to" ...
  if(curTok == Lex::tINTF) {
    intfs = readIntfs();
    checkFor(Lex::tTO);
  }
  return new Proc(intfs, readCmd());
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
    error("invalid command");
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
  case Lex::tSERVER:
  case Lex::tPROC:
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
  case Lex::tSERVER:
  case Lex::tPROC:
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
  // TODO: index-range = <expr>
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
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, &Syn::readFml);
}

// "(" {0 "," <decl> } ")"
inline std::list<Decl*> *Syn::readIntfs() {
  return readList<Decl>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, &Syn::readIntf);
}

// "[" {1 ":" <decl> } "]"
inline std::list<Spec*> *Syn::readHiddens() {
  return readList<Spec>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOLON, &Syn::readSpec);
}

// "(" {0 "," <expr> } ")"
inline std::list<Expr*> *Syn::readActuals() {
  return readList<Expr>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, &Syn::readExpr);
}

// "[" {1 "," <index-range> } "]"
inline std::list<Range*> *Syn::readRep() {
  return readList<Range>(
      Lex::tLSQ, Lex::tRSQ, Lex::tCOMMA, &Syn::readRange);
}

// "{" {0 "|" <choice> } "}"
inline std::list<Choice*> *Syn::readChoices() {
  return readList<Choice>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, &Syn::readChoice);
}

// "{" {0 "|" <altn> } "}"
inline std::list<Altn*> *Syn::readAltns() {
  return readList<Altn>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, &Syn::readAltn);
}

// "{" {0 "|" <select> } "}"
inline std::list<Select*> *Syn::readSelects() {
  return readList<Select>(
      Lex::tLCURLY, Lex::tRCURLY, Lex::tOR, &Syn::readSelect);
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
  // TODO
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
  // TODO
  return NULL;
}

// {1 "[" <expr> "]" }
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

