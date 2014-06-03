#include "Syn.h"
#include "Error.h"

#include <cassert>

Syn Syn::instance;

void Syn::getNextToken() {
  curTok = LEX.readToken();
}

void Syn::checkFor(Lex::Token t, const char *msg) {
  if(curTok != t)
    LEX.error(msg);
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

// program       = <spec> ":" <program>
//               | <seq>;
// specification = <decl>
//               | <abbr>
//               | <def>
// definition    = {0 "&" <def>}
Tree *Syn::readProg() {
  Tree *tree = new Tree();
  
  // Read specifications
  while(curTok == Lex::t_VAL 
     || curTok == Lex::t_VAR 
     || curTok == Lex::t_PROC 
     || curTok == Lex::t_SERV 
     || curTok == Lex::t_FUNC) {
    Spec *spec = readSpec();
    
    // Read a sequential specification
    switch(curTok) {
    default:
      error("bad separator for specification");
      break;
    // End of specification
    case Lex::t_COLON:
      tree->spec.push_back(spec);
      break;
    // Read simultaneous definitions
    case Lex::t_AND:
      if (spec->type != Spec::DEF)
        error("cannot make a simultaneous declaration");
      std::list<Def*> *defs = new std::list<Def*>();
      defs->push_back((Def *) spec);
      do {
        getNextToken();
        defs->push_back(readDef());
      } while (curTok == Lex::t_AND);
      SimDef *simDef = new SimDef(defs); 
      tree->spec.push_back(simDef);
      break;
    }
  }

  // Read program command sequence
  while (curTok != Lex::t_EOF)
    tree->prog.push_back(readCmd());
  
  return tree;
}

// specification = <decl>
//               | <abbr>
//               | <def>
Spec *Syn::readSpec() {
  switch(curTok) {
  default:            assert(0 && "invalid token");
  case Lex::t_EOF:  return NULL;
  case Lex::t_VAL:  return readValAbbr();
  case Lex::t_VAR:  return readDeclAbbr();
  case Lex::t_PROC: return readProc();
  case Lex::t_SERV: return readServ();
  case Lex::t_FUNC: return readFunc();
  }
}

// definition = "process" ...
//            | "server" ...
//            | "function" ...
Def *Syn::readDef() {
  switch(curTok) {
  default:            error("expecting definition");
  case Lex::t_PROC: return readProc();
  case Lex::t_SERV: return readServ();
  case Lex::t_FUNC: return readFunc();
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
  case Lex::t_COLON:
    res = new VarDecl(spef, name);
    break;

  // Name list declaration
  case Lex::t_COMMA:
    res = new VarDecl(spef, readNames());
    break;
  
  // Abbreviation
  case Lex::t_IS:
    res = new VarAbbr(spef, name, readElem());
    break;
  }

  checkFor(Lex::t_COLON, "expected ':' after declaration or abbreviation");
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
  case Lex::t_VAR:
    return new Spef(Spef::VAR, val, readDims());

  case Lex::t_CHAN:
    return new Spef(Spef::CHAN, val, readDims());

  case Lex::t_CALL:
    return new Spef(Spef::CALL, val, readDims());

  case Lex::t_INTF:
    return new IntfSpef(Spef::INTF, readList<Decl>(
          Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA,
          &Syn::readIntf), readDims());

  // <type> <name> [.][.]...[.]
  case Lex::t_PROC:
    return new NamedSpef(Spef::PROC, readName(), readDims());
  
  case Lex::t_SERV:
    return new NamedSpef(Spef::SERV, readName(), readDims());
  
  case Lex::t_FUNC:
    return new NamedSpef(Spef::FUNC, readName(), readDims());
  }
}

// "[" <expr> "]" "[" <expr> "]" ... "[" <expr> "]"
std::list<Expr*> *Syn::readDims() {
  if (curTok != Lex::t_LSQ) 
    return NULL;
  else {
    std::list<Expr*> *lengths = new std::list<Expr*>();
    while (curTok == Lex::t_LSQ) {
      lengths->push_back(readExpr());
      checkFor(Lex::t_RSQ, "']' missing");
      getNextToken();
    }
    return lengths;
  }
}

// def = process <name> "(" {0 "," <fml>} ")" "is" <cmd>
Def *Syn::readProc() {
  assert(curTok == Lex::t_PROC);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(
      Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readFml);
  checkFor(Lex::t_RPAREN, "')' missing");
  checkFor(Lex::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lex::t_INTF) {
    intfs = readList<Decl>(
        Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readIntf);
    checkFor(Lex::t_TO, "'to' missing");
  }
  return new Proc(name, args, intfs, readCmd());
}

// definition  = "server" <name> "(" {0, <fml>} ")" "inherits" <hiding-decl>
//             | "server" <name> "(" {0, <fml>} ")" "is" <cmd>
// hiding-decl = "from" { {1 : <decl> } } "interface" <name>
Def *Syn::readServ() {
  assert(curTok == Lex::t_SERV);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(
      Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readFml);
  checkFor(Lex::t_RPAREN, "')' missing");

  // Inheriting definition
  if(curTok == Lex::t_INHRT) {
    // TODO
    // return ...
  }

  checkFor(Lex::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lex::t_INTF) {
    intfs = readList<Decl>(
        Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readIntf);
    checkFor(Lex::t_TO, "'to' missing");
  }
  // read server decls
  return new Serv(name, args, intfs, NULL);
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Syn::readFunc() {
  assert(curTok == Lex::t_FUNC);
  getNextToken();
  Name *name = readName();
  checkFor(Lex::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(
      Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readFml);
  checkFor(Lex::t_RPAREN, "')' missing");
  checkFor(Lex::t_IS, "'is' missing");
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
    case Lex::t_CHAN: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      if (curTok != Lex::t_COMMA)
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
    case Lex::t_CALL: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      std::list<Fml*> *args = readList<Fml>(
          Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readFml);
      if (curTok != Lex::t_COMMA)
        return new CallDecl(spef, name, args);
      // Multiple calls
      else {
        std::list<Name*> *names = new std::list<Name*>();
        std::list<std::list<Fml*>*> *argss = new std::list<std::list<Fml*>*>(); 
        names->insert(names->begin(), name);
        while (curTok == Lex::t_COMMA) {
          getNextToken();
          names->push_back(readName());
          argss->push_back(readList<Fml>(
              Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readFml));
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
  if (curTok == Lex::t_VAL) {
    val = true;
    getNextToken();
  }
  switch(curTok) {
  default: 
      error("invalid argument");
      return NULL;
  case Lex::t_VAR:
  case Lex::t_INTF:
  case Lex::t_PROC:
  case Lex::t_SERV:
  case Lex::t_FUNC: {
      Spef *spef = readSpef(val);
      Name *name = readName();
      if (curTok != Lex::t_COMMA) 
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
  
  case Lex::t_SKIP:
    getNextToken();
    return new Skip();
  
  case Lex::t_STOP:
    getNextToken();
    return new Stop();

  case Lex::t_CONNECT: {
    getNextToken();
    Elem *source = readElem();
    checkFor(Lex::t_TO, "'to' missing");
    Elem *target = readElem();
    return new Connect(source, target);
  }

  // assignment, input, output, instance, call
  case Lex::t_NAME: {
    Name *name = readName();
    getNextToken();
    switch (curTok) {
    default:
      error("expecting assignment, input, output, instance or call");
      return NULL;
    
    case Lex::t_ASS:
      getNextToken();
      return new Ass(name, readExpr());
    
    case Lex::t_IN:
      getNextToken();
      return new In(name, readElem());
    
    case Lex::t_OUT:
      getNextToken();
      return new Out(name, readExpr());

    // Instance
    case Lex::t_LPAREN: {
      getNextToken();
      std::list<Expr*> *actuals = readList<Expr>(
          Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readExpr);
      checkFor(Lex::t_RPAREN, "')' missing");
      return new Instance(name, actuals);
    }
    
    // Call
    case Lex::t_DOT: {
        getNextToken();
        Name *field = readName();
        checkFor(Lex::t_LPAREN, "'(' missing");
        std::list<Expr*> *actuals = readList<Expr>(
            Lex::t_LPAREN, Lex::t_RPAREN, Lex::t_COMMA, &Syn::readExpr);
        checkFor(Lex::t_RPAREN, "')' missing");
        return new Call(name, field, actuals);
      }
    }
  } 

  // Structured commands
  // cond = "if" <expr> "do" <cmd>
  //      | "if" <expr> "then" <cmd> "else" <cmd>
  case Lex::t_IF: {
      getNextToken();
      Expr* expr = readExpr();
      switch(curTok) {
        default:
          error("expecting 'do' or 'then'");
          return NULL;

        case Lex::t_DO:
          return new IfD(expr, readCmd());
        
        case Lex::t_THEN: {
          Cmd *thenCmd = readCmd();
          checkFor(Lex::t_ELSE, "'else' missing");
          return new IfTE(expr, thenCmd, readCmd());
      }
      return NULL;
    }
  }

  // cond = "test" "{" {0 "|" <choice> } "}"
  //      | "test" <rep> <choice>
  case Lex::t_TEST:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;
  
    case Lex::t_LPAREN:
      return new Test(readList<Choice>(
            Lex::t_LSQ, Lex::t_RSQ, Lex::t_COMMA, &Syn::readChoice));
    
    case Lex::t_LSQ:
      return new RepTest(readRep(), readChoice());
    }

  // alt = "alt" "{" {0 "|" <altn> } "}"
  //     | "alt" <rep> <altn>
  case Lex::t_ALT:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;

    case Lex::t_LPAREN:
      return new Alt(readList<Altn>(
           Lex::t_LSQ, Lex::t_RSQ, Lex::t_COMMA, &Syn::readAltn));
    
    case Lex::t_LSQ:
      return new RepAlt(readRep(), readAltn());
    }

  // case = "case" <expr> "{" {0 "|" <selection> } "}"
  //      | "alt" <rep> <selection>
  case Lex::t_CASE:
    // TODO
    return NULL;

  // loop = "while" <expr> "do" <cmd>
  case Lex::t_WHILE:
    // TODO
    return NULL;

  // loop = "do" <cmd> "while" <expr>
  case Lex::t_DO:
    // TODO
    return NULL;

  // loop = "until" <expr> "do" <cmd>
  case Lex::t_UNTIL:
    // TODO
    return NULL;

  // Declaration or abbreviation
  case Lex::t_VAL: 
  case Lex::t_VAR:
  case Lex::t_CHAN: 
  case Lex::t_CALL: 
  case Lex::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lex::t_COLON, "':' missing");
    return new CmdSpec(spec, readCmd());
  }

  // Disallowed
  case Lex::t_PROC:
  case Lex::t_SERV:
  case Lex::t_FUNC:
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
  case Lex::t_TEST:
    return new NestedChoice((Test*) readCmd());

  // Choice specification
  case Lex::t_VAL:
  case Lex::t_VAR: 
  case Lex::t_CHAN: 
  case Lex::t_CALL: 
  case Lex::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lex::t_COLON, "':' missing");
    return new SpecChoice(spec, readChoice());
  }
  
  // Disallowed specifications
  case Lex::t_PROC:
  case Lex::t_SERV:
  case Lex::t_FUNC:
    error("definition in specification of choice");
    readSpec();
    return new SpecChoice(NULL, readChoice());
  }

  // Choice (assume <expr>)
  Expr *expr = readExpr();
  checkFor(Lex::t_COLON, "':' missing");
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
  case Lex::t_ALT:
    return new NestedAltn((Alt*) readCmd());

  // Alternative specification
  case Lex::t_VAL:
  case Lex::t_VAR: 
  case Lex::t_CHAN: 
  case Lex::t_CALL: 
  case Lex::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lex::t_COLON, "':' missing");
    return new SpecAltn(spec, readAltn());
  }
  
  // Disallowed specifications
  case Lex::t_PROC:
  case Lex::t_SERV:
  case Lex::t_FUNC:
    error("definition in specification of alternative");
    readSpec();
    return new SpecAltn(NULL, readAltn());
  }

  // Alternative
  // TODO: unguarded, guarded, skip
  //Expr *expr = readExpr();
  //checkFor(Lex::t_COLON, "':' missing");
  //return new Altn(expr, elem, elem, readCmd());
  return NULL;
}

// index-range = <name> "=" <expr> "for" <expr>
//             | <name> "=" <expr> "for" <expr> "step" <expr>
IndexRange *Syn::readIndexRange() {
  Name *name = readName();
  checkFor(Lex::t_EQ, "'=' missing");
  Expr *base = readExpr();
  checkFor(Lex::t_FOR, "'for' missing");
  Expr *count = readExpr();
  Expr *step = NULL;
  if (curTok == Lex::t_STEP) {
    getNextToken();
    step = readExpr();
  }
  return new IndexRange(name, base, count, step);
}

// Read a list of T
// <left> {0 <sep> <item> } <right>
// item = Fml | Decl | Expr | IndexRange
template<typename T>
std::list<T*> *Syn::readList(
    Lex::Token left, 
    Lex::Token right, 
    Lex::Token sep, 
    T *(Syn::*readItem)()) {
  checkFor(left, "'" Lex::tokStr(left) "' missing");
  std::list<T*> *l = new std::list<T*>();
  do {
    l->push_back((this->*readItem)());
    getNextToken();
  } while (curTok != sep);
  checkFor(right, "'" Lex::tokStr(right) "' missing");
  return l;
}

// elem = ...
Elem *Syn::readElem() {
  return NULL;
}

// <name>
Name *Syn::readName() {
  std::string *s = &LEX.s;
  checkFor(Lex::t_NAME, "name expected");
  return new Name(*s);
}

// expr = ...
Expr *Syn::readExpr() {
  return NULL;
}

// {0 "," <name> }
std::list<Name*> *Syn::readNames() {
  std::list<Name*> *names = new std::list<Name*>();
  do {
    getNextToken();
    // In formal lists, don't read next specifier
    if (curTok != Lex::t_NAME)
      break;
    names->push_back(readName());
  } while (curTok != Lex::t_COMMA);
  return names;
}

