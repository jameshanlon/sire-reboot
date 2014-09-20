#include "Syn.h"
#include "Error.h"

#include <stdio.h>
#include <cassert>

Syn Syn::instance;

void Syn::getNextToken() {
  curTok = LEX.readToken();
  LEX.printToken(curTok);
}

void Syn::checkFor(Lex::Token t) {
  if(curTok != t) {
    char msg[25];
    sprintf(msg, "'%s' expected", LEX.tokStr(t));
    LEX.error(msg);
  }
  getNextToken();
}

void Syn::error(const char *msg) {
  LEX.error(msg);
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
//         | {0 "&" <spec> }
Tree *Syn::readProg() {
  Tree *tree = new Tree();
  getNextToken();

  // Read specifications
  while (curTok == Lex::tVAL
      || curTok == Lex::tVAR
      || curTok == Lex::tCHAN
      || curTok == Lex::tCALL
      || curTok == Lex::tPROCESS
      || curTok == Lex::tSERVER
      || curTok == Lex::tFUNCTION) {

    // ... <spec> ":"
    tree->spec.push_back(readSpec());
    checkFor(Lex::tCOLON);
  }

  // ... {1 ";" <cmd> }
  while (curTok != Lex::tEOF) {
    tree->prog.push_back(readCmd());
    if (curTok == Lex::tSEMI)
      getNextToken();
    else
      break;
  }

  return tree;
}

// Specifier ==================================================================

// spef = <type>
//      | <type> <name>
//      | <type> <interface>
//      | <spef> "[" "]"
//      | <spef> "[" <expr> "]"
// type = "var"
//      | "chan"
//      | "call"
//      | "function"
//      | "process"
//      | "server"
Spef *Syn::readSpef(bool val) {
  Lex::Token t = curTok;
  getNextToken();
  switch (t) {
  default:
    error("invalid specifier");
    return nullptr;

  // <type> {0, "[" <expr> "]" }
  case Lex::tVAR:
    return new Spef(Spef::VAR, val, readDims());

  case Lex::tCHAN:
    return new Spef(Spef::CHAN, val, readDims());

  case Lex::tCALL:
    return new Spef(Spef::CALL, val, readDims());

  case Lex::tFUNCTION:
    return new Spef(Spef::FUNCTION, val, readDims());

  // "process" <name> {0 "[" <expr> "]" }
  // "process" <interface> {0 "[" <expr> "]" }
  case Lex::tPROCESS:
    switch (curTok) {
    default:
      error("expecting name or 'interface'");
      return nullptr;

    case Lex::tNAME:
      return new NamedSpef(Spef::PROCESS, readName(), readDims());

    case Lex::tINTF:
      return new IntfSpef(Spef::PROCESS, readIntfs(), readDims());
    }

  // "server" <name> {0 "[" <expr> "]" }
  // "server" <interface> {0 "[" <expr> "]" }
  case Lex::tSERVER:
    switch (curTok) {
    default:
      error("expecting name or 'interface'");
      return nullptr;

    case Lex::tNAME:
      return new NamedSpef(Spef::SERVER, readName(), readDims());

    case Lex::tINTF:
      return new IntfSpef(Spef::SERVER, readIntfs(), readDims());
    }
  }
}

// Specification ==============================================================

// spec        = <decl>
//             | <abbr>
//             | <def>
//             | {0 "&" <spec> }
// decl        = <spef> {1 "," <name> }
//             | <server-decl>
//             | <hiding-decl>
// abbr        = <spef> <name> "is" <elem>
//             | "val" <spef> <name> "is" <elem>
// def         = <server-def>
//             | <process-def>
//             | <function-def>
Spec *Syn::readSpec() {
  Spec *res;
  switch(curTok) {
  default:
    assert(0 && "invalid token");

  case Lex::tEOF:
    return nullptr;

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
  case Lex::tSERVER:
    res = readServerSpec();
    break;

  // def  = "process" ...
  // abbr = "process" ...
  case Lex::tPROCESS:
    res = readProcessSpec();
    break;

  // def  = "function" ...
  // abbr = "function" ...
  case Lex::tFUNCTION:
    res = readFunctionSpec();
    break;
  }

  // Separator
  switch (curTok) {
  default:
    error("expected ':' or '&'");
    return nullptr;

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

// "server" ...
// def         = "server" <name> "(" {0 "," <fml> } ")" ...
// decl        = "server" <name> "is" <server>
//             | "server" <name> "is" <rep> <server>
// abbr        = <server-spef> <name> "is" <elem>
// server-spef = "server" <name>
//             | "server" <interface>
//             | <server-spef> "[" "]"
//             | <server-spef> "[" <expr> "]"
Spec *Syn::readServerSpec() {

  checkFor(Lex::tSERVER);
  switch(curTok) {
  default:
    error("expected name or 'interface'");
    return nullptr;

  // "server" <name> ...
  case Lex::tNAME: {
      Name *name = readName();
      switch(curTok) {
      default:
        error("expected '(' or 'is'");
        return nullptr;

      // Definition
      // ... "(" {0 "," <fml> } ")" "is" <server>
      // ... "(" {0 "," <fml> } ")" "inherits" <hiding>
      case Lex::tLPAREN: {
          std::list<Fml*> *args = readFmls();
          switch(curTok) {
          default:
            error("expecting 'is' or 'inherits'");
            return nullptr;

          // ... "is" <server>
          case Lex::tIS:
            return new ServerDef(name, args, readServer());

          // ... "inherits" <hiding-decl>
          case Lex::tINHRT:
            getNextToken();
            return new InhrtServerDef(name, args, readHiding());
          }
        }

      // Abbreviation
      // ... {1 "[" <expr>? "]" } <name> "is" <elem>
      case Lex::tLSQ: {
          Spef *spef = new NamedSpef(Spef::SERVER, name, readDims());
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ServerAbbr(spef, name, readElem());
        }

      // Declaration or abbreviation
      // ... "is" ...
      case Lex::tIS:
        getNextToken();
        switch(curTok) {
        default:
          error("expected '[', 'interface', 'inherits' or instance");
          return nullptr;

        // Declaration
        // ... <server>
        case Lex::tINTF:
          return new ServerDecl(name, readServer());

        // Replicated declaration
        // ... <rep> <server>
        case Lex::tLSQ:
          return new RServerDecl(name, readRep(), readServer());

        // Declaration or abbreviation
        // ... <name> "(" {0 "," <expr>? } ")"
        // ... <name>
        case Lex::tNAME: {
            Name *server = readName();
            if (curTok == Lex::tLPAREN) {
              std::list<Expr*> *args = readActuals();
              return new ServerDecl(name, new ServerInstance(server, args));
            }
            else {
              Spef *spef = new NamedSpef(Spef::SERVER, name);
              return new ServerAbbr(spef, name, readElem());
            }
          }
        }
      }
    }

  // Abbreviation
  // "server" "interface" "(" {0 "," <decl> } ")" ...
  case Lex::tINTF: {
      getNextToken();
      std::list<Decl*> *intfs = readIntfs();
      switch (curTok) {
      default:
        error("expected name or '['");
        return nullptr;

      // ... <name> "is" <elem>
      case Lex::tNAME: {
          Spef *spef = new IntfSpef(Spef::SERVER, intfs);
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ServerAbbr(spef, name, readElem());
        }

      // ... {1 "[" <expr>? "]" } <name> "is" <elem>
      case Lex::tLSQ: {
          Spef *spef = new IntfSpef(Spef::SERVER, intfs, readDims());
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ServerAbbr(spef, name, readElem());
        }
      }
    }
  }
}

// "process" ...
// def          = "process" <name> "(" {0 "," <fml> } ")" "is" <process>
// abbr         = <process-spef> <name> "is" <elem>
// process-spef = "process" <name>
//              | "process" <interface>
//              | <proc-spef> "[" "]"
//              | <proc-spef> "[" <expr> "]"
Spec *Syn::readProcessSpec() {
  checkFor(Lex::tPROCESS);
  switch (curTok) {
  default:
    error("expecting name or 'interface'");
    return nullptr;

  // Declaration or abbreviation
  // "process" <name> ...
  case Lex::tNAME: {
      Name *name = readName();
      switch(curTok) {
      default:
        error("expected '(', 'is' or '['");
        return nullptr;

      // Definition
      // ... "(" {0 "," <fml> } ")" "is" <process>
      case Lex::tLPAREN: {
          std::list<Fml*> *args = readFmls();
          checkFor(Lex::tIS);
          return new ProcessDef(name, args, readProcess());
        }

      // Abbreviation
      // ... "is" <elem>
      case Lex::tIS: {
          getNextToken();
          Spef *spef = new NamedSpef(Spef::PROCESS, name);
          return new ProcessAbbr(spef, name, readElem());
        }

      // Abbreviation
      // ... {1 "[" <expr>? "]" } <name> "is" <elem>
      case Lex::tLSQ: {
          Spef *spef = new NamedSpef(Spef::PROCESS, name, readDims());
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ProcessAbbr(spef, name, readElem());
        }
      }
    }

  // Abbreviation
  // "process" "interface" "(" {0 "," <decl> } ")" ...
  case Lex::tINTF: {
      getNextToken();
      std::list<Decl*> *intfs = readIntfs();
      switch (curTok) {
      default:
        error("expected name or '['");
        return nullptr;

      // ... <name> "is" <elem>
      case Lex::tNAME: {
          Spef *spef = new IntfSpef(Spef::PROCESS, intfs);
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ServerAbbr(spef, name, readElem());
        }

      // ... {1 "[" <expr>? "]" } <name> "is" <elem>
      case Lex::tLSQ: {
          Spef *spef = new IntfSpef(Spef::PROCESS, intfs, readDims());
          Name *name = readName();
          checkFor(Lex::tIS);
          return new ServerAbbr(spef, name, readElem());
        }
      }
    }
  }
}

// "function" ...
// def  = "function" <name> "(" {0 "," <fml> } ")" "is" <expr>
// abbr = <function-spef> <name> "is" <elem>
// function-spef = "function"
//               | <function-spef> "[" "]"
//               | <function-spef> "[" <expr> "]"
Spec *Syn::readFunctionSpec() {
  checkFor(Lex::tFUNCTION);
  switch (curTok) {
  default:
    error("expecting name or '['");
    return nullptr;

  // "function" <name> ...
  case Lex::tNAME: {
      Name *name = readName();
      switch(curTok) {
      default:
        error("expecting '(' or 'is'");
        return nullptr;

      // Definition
      // ... "(" {0 "," <fml> } ")" "is" <expr>
      case Lex::tLPAREN: {
          std::list<Fml*> *args = readFmls();
          checkFor(Lex::tIS);
          return new FunctionDef(name, args, readExpr());
        }

      // Abbreviation
      // ... "is" <elem>
      case Lex::tIS: {
          getNextToken();
          Spef *spef = new Spef(Spef::FUNCTION);
          return new FunctionAbbr(spef, name, readElem());
        }
      }
    }

  // "function" {1 "[" <expr> "]" } <name> "is" <expr>
  case Lex::tLSQ: {
      Spef *spef = new Spef(Spef::FUNCTION, readDims());
      Name *name = readName();
      checkFor(Lex::tIS);
      return new FunctionAbbr(spef, name, readElem());
    }
  }
}

// decl = <hiding>
// hiding = "from" "[" {1 ":" <decl> } "]" "interface" <elem>
Hiding *Syn::readHiding() {
  checkFor(Lex::tFROM);
  std::list<Spec*> *decls = readHiddens();
  checkFor(Lex::tINTF);
  return new Hiding(readName(), decls);
}

// def         = "server" <name> "(" {0, <fml> } ")" "is" <server>
//             | "server" <name> "(" {0, <fml> } ")" "inherits" <hiding>
// server      = <interface> "to" <server-spec>
// server-spec = <decl>
//             | "{" {0 ":" <decl> "}"
// hiding      = "from" "[" {1 "," <decl> } "]" "interface" <name>
Def *Syn::readServerDef() {

  // "server" <name> "(" {0, <fml>} ")" ...
  checkFor(Lex::tSERVER);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();

  switch(curTok) {
  default:
    error("expecting 'is or 'inherits'");
    return nullptr;

  // ... "is" ...
  case Lex::tIS:
    getNextToken();
    return new ServerDef(name, args, readServer());

  // ... "inherits" <hiding-decl>
  case Lex::tINHRT:
    getNextToken();
    return new InhrtServerDef(name, args, readHiding());
  }
}

// def = "process" <name> "(" {0 "," <fml> } ")" "is" <process>
Def *Syn::readProcessDef() {
  checkFor(Lex::tPROCESS);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  return new ProcessDef(name, args, readProcess());
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Syn::readFunctionDef() {
  checkFor(Lex::tFUNCTION);
  Name *name = readName();
  std::list<Fml*> *args = readFmls();
  checkFor(Lex::tIS);
  return new FunctionDef(name, args, readExpr());
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
      return nullptr;

  // "val" ...
  // "interface" ...
  // "process" ...
  // "server" ...
  // "function" ...
  case Lex::tVAR:
  case Lex::tCHAN:
  case Lex::tSERVER:
  case Lex::tPROCESS:
  case Lex::tFUNCTION: {
      Spef *spef = readSpef(val);
      Name *name = readName();
      return new Fml(spef, name);
    }
  }
}

// Servers and processes ======================================================

// <decl>
// decl = "call" {1 "," <name> "(" <fml> ")" }
Decl *Syn::readIntf() {
  switch(curTok) {
    default:
      error("invalid interface");
      return nullptr;

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

// server = <interface> "to" <decl>
//        | <interface> "to" "{" {0 ":" <decl> } "}"
//        | <instance>
Server *Syn::readServer() {

  // Instance
  // <name> "(" {0 "," <actual> } ")"
  if (curTok == Lex::tNAME)
    return new ServerInstance(readName(), readActuals());

  // Specification
  // "interface" "(" {0 "," <decl> } ")" "to" ...
  checkFor(Lex::tINTF);
  std::list<Decl*> *intfs = readIntfs();
  checkFor(Lex::tTO);

  // ... "{" {0 ":" <decl> } "}"
  if (curTok == Lex::tLCURLY)
    return new ServerSpec(intfs, readSpecs());
  else {
    std::list<Spec*> *specs = new std::list<Spec*>();
    specs->push_back(readSpec());
    return new ServerSpec(intfs, specs);
  }
}

// process = <cmd>
//         | <interface> "to" <cmd>
//         | <instance>
Process *Syn::readProcess() {

  // Instance
  // <name> "(" {0 "," <actual> } ")"
  if (curTok == Lex::tNAME)
    return new ProcessInstance(readName(), readActuals());

  // Speficiation
  // "interface" "(" {0 "," <decl> } ")" "to" <cmd>
  if (curTok == Lex::tINTF) {
    getNextToken();
    std::list<Decl*> *intfs = readIntfs();
    checkFor(Lex::tTO);
    return new ProcessSpec(intfs, readCmd());
  }

  // <cmd>
  return new ProcessCmd(readCmd());
}

// Commands ===================================================================

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
    return nullptr;

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
      return nullptr;

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
          return nullptr;

        case Lex::tDO:
          return new IfD(expr, readCmd());

        case Lex::tTHEN: {
          Cmd *thenCmd = readCmd();
          checkFor(Lex::tELSE);
          return new IfTE(expr, thenCmd, readCmd());
      }
      return nullptr;
    }
  }

  // cond = "test" "{" {0 "|" <choice> } "}"
  //      | "test" <rep> <choice>
  case Lex::tTEST:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return nullptr;

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
      return nullptr;

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
      return nullptr;

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
  case Lex::tCALL: {
    Spec *spec = readSpec();
    checkFor(Lex::tCOLON);
    return new CmdSpec(spec, readCmd());
  }

  // Disallowed
  case Lex::tSERVER:
  case Lex::tPROCESS:
  case Lex::tFUNCTION:
    error("definition in specification of command");
    readSpec();
    return new CmdSpec(nullptr, readCmd());
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
  case Lex::tPROCESS:
  case Lex::tFUNCTION:
    error("definition in specification of choice");
    readSpec();
    return new SpecChoice(nullptr, readChoice());
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
  case Lex::tPROCESS:
  case Lex::tSERVER:
  case Lex::tFUNCTION:
    error("definition in specification of alternative");
    readSpec();
    return new SpecAltn(nullptr, readAltn());
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

// range = <name> "=" <expr> "for" <expr>
//       | <name> "=" <expr> "for" <expr> "step" <expr>
Range *Syn::readRange() {
  // TODO: range = <expr>
  Name *name = readName();
  checkFor(Lex::tEQ);
  Expr *base = readExpr();
  checkFor(Lex::tFOR);
  Expr *count = readExpr();
  Expr *step = nullptr;
  if (curTok == Lex::tSTEP) {
    getNextToken();
    step = readExpr();
  }
  return new Range(name, base, count, step);
}

// Lists ======================================================================

// {1 "[" <expr> "]" }
std::list<Expr*> *Syn::readDims() {
  if (curTok != Lex::tLSQ)
    return nullptr;
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

// "(" {0 "," <decl> } ")"
inline std::list<Spec*> *Syn::readSpecs() {
  return readList<Spec>(
      Lex::tLPAREN, Lex::tRPAREN, Lex::tCOMMA, &Syn::readSpec);
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

// "[" {1 "," <range> } "]"
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
  if (curTok == right) {
    getNextToken();
    return nullptr;
  }
  std::list<T*> *l = new std::list<T*>();
  while (true) {
    l->push_back((this->*readItem)());
    if (curTok == sep)
      getNextToken();
    else
      break;
  }
  checkFor(right);
  return l;
}

// Elements ===================================================================

// elem  = <elem> "[" <expr> "]"
//       | <field>
//       | <name>
// field = <elem> "." <name>
Elem *Syn::readElem() {
  Name *name = readName();

  // Field
  if (curTok == Lex::tDOT) {
    getNextToken();
    Name *field = readName();
    if (curTok == Lex::tLSQ)
      return new Field(name, field, readDims());
    return new Field(name, field);
  }

  // Name
  if (curTok == Lex::tLSQ)
    name->subscripts = readDims();
  return name;
}

// <name>
Name *Syn::readName() {
  std::string *s = &LEX.s;
  checkFor(Lex::tNAME);
  return new Name(s);
}

// Expressions ================================================================

// expr    = <op> <operand>
//         | <operand> <op> <operand>
//         | <operand>
Expr *Syn::readExpr() {
  switch (curTok) {

  // Unary not
  case Lex::tNOT:
    getNextToken();
    return new UnaryOp(Lex::tNOT, readOperand());

  // Unary minus
  case Lex::tSUB:
    getNextToken();
    return new UnaryOp(Lex::tSUB, readOperand());

  // <operand> | <operand> <op> <operand>
  default:
    Operand *operand = readOperand();
    if (isOp(curTok)) {
      Lex::Token op = curTok;
      getNextToken();
      return new BinaryOp(op, operand, readOperand());
    }
    else
      return operand;
  }
}

bool Syn::isOp(Lex::Token) {
  switch (curTok) {
  default:
    return false;
  case Lex::tADD: case Lex::tOR:   case Lex::tEQ:
  case Lex::tSUB: case Lex::tAND:  case Lex::tLT:
  case Lex::tMUL: case Lex::tLAND: case Lex::tGT:
  case Lex::tDIV: case Lex::tLOR:  case Lex::tNEQ:
  case Lex::tREM: case Lex::tXOR:  case Lex::tLEQ:
                  case Lex::tLSH:  case Lex::tGEQ:
                  case Lex::tRSH:
    return true;
  }
}

// operand = <elem>
//         | <literal>
//         | <valof>
//         | "(" <expr> ")"
// literal = <decint>
//         | "0x" <hexint>
//         | "0o" <octint>
//         | "0b" <binint>
//         | <byte>
//         | "true"
//         | "false"
// byte    = "'" <char> "'"
Operand *Syn::readOperand() {
  switch (curTok) {
  default:
    error("expecing name, 'valof' or literal");
    return nullptr;

  // <elem>
  case Lex::tNAME:
    return new OperElem(readElem());

  // <valof>
  case Lex::tVALOF:
    return new OperValof(readValof());

  // "(" <expr> ")"
  case Lex::tLPAREN: {
      getNextToken();
      Expr *expr = readExpr();
      checkFor(Lex::tRPAREN);
      return new OperExpr(expr);
    }

  // Literal <decint>
  case Lex::tDECINT:
    return new OperLiteral(new DecIntLiteral(LEX.value));

  // Literal <hexint>
  case Lex::tHEXINT:
    return new OperLiteral(new HexIntLiteral(LEX.value));

  // Literal <octint>
  case Lex::tOCTINT:
    return new OperLiteral(new OctIntLiteral(LEX.value));

  // Literal <binint>
  case Lex::tBININT:
    return new OperLiteral(new BinIntLiteral(LEX.value));

  // Literal <char>
  case Lex::tCHAR:
    return new OperLiteral(new CharLiteral(LEX.value));

  // Literal "true"
  case Lex::tTRUE:
    return new OperLiteral(new BoolLiteral(true));

  // Literal "false"
  case Lex::tFALSE:
    return new OperLiteral(new BoolLiteral(false));
  }
}

// valof = "valof" <cmd> "result" <expr>
Valof *Syn::readValof() {
  checkFor(Lex::tVALOF);
  Cmd *cmd = readCmd();
  checkFor(Lex::tRESULT);
  return new Valof(cmd, readExpr());
}

