#include "Parser.h"
#include "Error.h"

#include <cassert>

Parser Parser::instance;

void Parser::getNextToken() {
  curTok = LEX.readToken();
}

void Parser::checkFor(Lexer::Token t, const char *msg) {
  if(curTok != t)
    LEX.error(msg);
  LEX.readToken();
}

void Parser::error(const char *msg) {
  LEX.error(msg);
  getNextToken();
}

Tree *Parser::formTree() {
  Tree *t = readProg();
  return t;
}

// program       = <spec> ":" <program>
//               | <seq>;
// specification = <decl>
//               | <abbr>
//               | <def>
// definition    = {0 "&" <def>}
Tree *Parser::readProg() {
  Tree *tree = new Tree();
  
  // Read specifications
  while(curTok == Lexer::t_VAL 
     || curTok == Lexer::t_VAR 
     || curTok == Lexer::t_PROC 
     || curTok == Lexer::t_SERV 
     || curTok == Lexer::t_FUNC) {
    Spec *spec = readSpec();
    
    // Read a sequential specification
    switch(curTok) {
    default:
      error("bad separator for specification");
      break;
    // End of specification
    case Lexer::t_COLON:
      tree->spec.push_back(spec);
      break;
    // Read simultaneous definitions
    case Lexer::t_AND:
      if (spec->type != Spec::DEF)
        error("cannot make a simultaneous declaration");
      std::list<Def*> *defs = new std::list<Def*>();
      defs->push_back((Def *) spec);
      do {
        getNextToken();
        defs->push_back(readDef());
      } while (curTok == Lexer::t_AND);
      SimDef *simDef = new SimDef(defs); 
      tree->spec.push_back(simDef);
      break;
    }
  }

  // Read program command sequence
  while (curTok != Lexer::t_EOF)
    tree->prog.push_back(readCmd());
  
  return tree;
}

// specification = <decl>
//               | <abbr>
//               | <def>
Spec *Parser::readSpec() {
  switch(curTok) {
  default:            assert(0 && "invalid token");
  case Lexer::t_EOF:  return NULL;
  case Lexer::t_VAL:  return readValAbbr();
  case Lexer::t_VAR:  return readDeclAbbr();
  case Lexer::t_PROC: return readProc();
  case Lexer::t_SERV: return readServ();
  case Lexer::t_FUNC: return readFunc();
  }
}

// definition = "process" ...
//            | "server" ...
//            | "function" ...
Def *Parser::readDef() {
  switch(curTok) {
  default:            error("expecting definition");
  case Lexer::t_PROC: return readProc();
  case Lexer::t_SERV: return readServ();
  case Lexer::t_FUNC: return readFunc();
  }
}

// abbr = <spef> <name> "is" <elem>
//      | "call" <name> "(" {0 "," <fml> } ")" "is" <name>
Abbr *Parser::readValAbbr() {
  getNextToken();
  Spef *s = readSpef(true);
  Name *n = readName();
  return new VarAbbr(s, n, readElem()); 
}

// spec = <decl>
//      | <abbr>
// decl = <spec> {1 "," <name> }
// abbr = <spec> <name> "is" <element>
Spec *Parser::readDeclAbbr() {
  Spef *spef = readSpef(false);
  Name *name = readName();
  Spec *res;
  switch (curTok) {
  default: assert(0 && "invalid token");
  
  // Single-name declaration
  case Lexer::t_COLON:
    res = new VarDecl(spef, name);
    break;

  // Name list declaration
  case Lexer::t_COMMA:
    res = new VarDecl(spef, readNames());
    break;
  
  // Abbreviation
  case Lexer::t_IS:
    res = new VarAbbr(spef, name, readElem());
    break;
  }

  checkFor(Lexer::t_COLON, "expected ':' after declaration or abbreviation");
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
Spef *Parser::readSpef(bool val) {
  Lexer::Token t = curTok;
  getNextToken();
  switch (t) {
  default: 
    error("invalid specifier");
    return NULL;
 
  // <type> [.][.]...[.]
  case Lexer::t_VAR:
    return new Spef(Spef::VAR, val, readDims());

  case Lexer::t_CHAN:
    return new Spef(Spef::CHAN, val, readDims());

  case Lexer::t_CALL:
    return new Spef(Spef::CALL, val, readDims());

  case Lexer::t_INTF:
    return new IntfSpef(Spef::INTF, readList<Decl>(&Parser::readIntf), readDims());

  // <type> <name> [.][.]...[.]
  case Lexer::t_PROC:
    return new NamedSpef(Spef::PROC, readName(), readDims());
  
  case Lexer::t_SERV:
    return new NamedSpef(Spef::SERV, readName(), readDims());
  
  case Lexer::t_FUNC:
    return new NamedSpef(Spef::FUNC, readName(), readDims());
  }
}

// "[" <expr> "]" "[" <expr> "]" ... "[" <expr> "]"
std::list<Expr*> *Parser::readDims() {
  if (curTok != Lexer::t_LSQ) 
    return NULL;
  else {
    std::list<Expr*> *lengths = new std::list<Expr*>();
    while (curTok == Lexer::t_LSQ) {
      lengths->push_back(readExpr());
      checkFor(Lexer::t_RSQ, "']' missing");
      getNextToken();
    }
    return lengths;
  }
}

// def = process <name> "(" {0 "," <fml>} ")" "is" <cmd>
Def *Parser::readProc() {
  assert(curTok == Lexer::t_PROC);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(&Parser::readFml);
  checkFor(Lexer::t_RPAREN, "')' missing");
  checkFor(Lexer::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lexer::t_INTF) {
    intfs = readList<Decl>(&Parser::readIntf);
    checkFor(Lexer::t_TO, "'to' missing");
  }
  return new Proc(name, args, intfs, readCmd());
}

// definition  = "server" <name> "(" {0, <fml>} ")" "inherits" <hiding-decl>
//             | "server" <name> "(" {0, <fml>} ")" "is" <cmd>
// hiding-decl = "from" { {1 : <decl> } } "interface" <name>
Def *Parser::readServ() {
  assert(curTok == Lexer::t_SERV);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(&Parser::readFml);
  checkFor(Lexer::t_RPAREN, "')' missing");

  // Inheriting definition
  if(curTok == Lexer::t_INHRT) {
    // TODO
    // return ...
  }

  checkFor(Lexer::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lexer::t_INTF) {
    intfs = readList<Decl>(&Parser::readIntf);
    checkFor(Lexer::t_TO, "'to' missing");
  }
  // read server decls
  return new Serv(name, args, intfs, NULL);
}

// def = "function" <name> "(" {0 "," <fml>} ")" "is" <expr>
Def *Parser::readFunc() {
  assert(curTok == Lexer::t_FUNC);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readList<Fml>(&Parser::readFml);
  checkFor(Lexer::t_RPAREN, "')' missing");
  checkFor(Lexer::t_IS, "'is' missing");
  return new Func(name, args, readExpr());
}

// <decl>
// decl = "call" {1 "," <name> "(" <fml> ")" }
Decl *Parser::readIntf() {
  switch(curTok) {
    default: 
      error("invalid interface");
      return NULL;

    // Chanend interface
    case Lexer::t_CHAN: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      if (curTok != Lexer::t_COMMA)
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
    case Lexer::t_CALL: {
      Spef *spef = readSpef(false);
      Name *name = readName();
      std::list<Fml*> *args = readList<Fml>(&Parser::readFml);
      if (curTok != Lexer::t_COMMA)
        return new CallDecl(spef, name, args);
      // Multiple calls
      else {
        std::list<Name*> *names = new std::list<Name*>();
        std::list<std::list<Fml*>*> *argss = new std::list<std::list<Fml*>*>(); 
        names->insert(names->begin(), name);
        while (curTok == Lexer::t_COMMA) {
          getNextToken();
          names->push_back(readName());
          argss->push_back(readList<Fml>(&Parser::readFml));
        }
        return new CallDecl(spef, names, argss);
      }
    }
  }
}

// fml = <spef> {1 "," <name> }
//     | "val" <spef> {1 "," <name> }
Fml *Parser::readFml() {
  bool val = false;
  if (curTok == Lexer::t_VAL) {
    val = true;
    getNextToken();
  }
  switch(curTok) {
  default: 
      error("invalid argument");
      return NULL;
  case Lexer::t_VAR:
  case Lexer::t_INTF:
  case Lexer::t_PROC:
  case Lexer::t_SERV:
  case Lexer::t_FUNC: {
      Spef *spef = readSpef(val);
      Name *name = readName();
      if (curTok != Lexer::t_COMMA) 
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
Cmd *Parser::readCmd() {
  switch(curTok) {
  default: 
    error("bad command");
    return NULL;
  
  case Lexer::t_SKIP:
    getNextToken();
    return new Skip();
  
  case Lexer::t_STOP:
    getNextToken();
    return new Stop();

  case Lexer::t_CONNECT: {
    getNextToken();
    Elem *source = readElem();
    checkFor(Lexer::t_TO, "'to' missing");
    Elem *target = readElem();
    return new Connect(source, target);
  }

  // assignment, input, output, instance, call
  case Lexer::t_NAME: {
    Name *name = readName();
    getNextToken();
    switch (curTok) {
    default:
      error("expecting assignment, input, output, instance or call");
      return NULL;
    
    case Lexer::t_ASS:
      getNextToken();
      return new Ass(name, readExpr());
    
    case Lexer::t_IN:
      getNextToken();
      return new In(name, readElem());
    
    case Lexer::t_OUT:
      getNextToken();
      return new Out(name, readExpr());

    // Instance
    case Lexer::t_LPAREN: {
      getNextToken();
      std::list<Expr*> *actuals = readList<Expr>(&Parser::readExpr);
      checkFor(Lexer::t_RPAREN, "')' missing");
      return new Instance(name, actuals);
    }
    
    // Call
    case Lexer::t_DOT: {
        getNextToken();
        Name *field = readName();
        checkFor(Lexer::t_LPAREN, "'(' missing");
        std::list<Expr*> *actuals = readList<Expr>(&Parser::readExpr);
        checkFor(Lexer::t_RPAREN, "')' missing");
        return new Call(name, field, actuals);
      }
    }
  } 

  // Structured commands
  // cond = "if" <expr> "do" <cmd>
  //      | "if" <expr> "then" <cmd> "else" <cmd>
  case Lexer::t_IF: {
      getNextToken();
      Expr* expr = readExpr();
      switch(curTok) {
        default:
          error("expecting 'do' or 'then'");
          return NULL;

        case Lexer::t_DO:
          return new IfD(expr, readCmd());
        
        case Lexer::t_THEN: {
          Cmd *thenCmd = readCmd();
          checkFor(Lexer::t_ELSE, "'else' missing");
          return new IfTE(expr, thenCmd, readCmd());
      }
      return NULL;
    }
  }

  // cond = "test" "{" {0 "|" <choice> } "}"
  //      | "test" <rep> <choice>
  case Lexer::t_TEST:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;
  
    case Lexer::t_LPAREN:
      return new Test(readOptions<Choice>(&Parser::readChoice));
    
    case Lexer::t_LSQ:
      return new RepTest(readRep(), readChoice());
    }

  // alt = "alt" "{" {0 "|" <altn> } "}"
  //     | "alt" <rep> <altn>
  case Lexer::t_ALT:
    getNextToken();
    switch (curTok) {
    default:
      error("expecting '{' or '['");
      return NULL;

    case Lexer::t_LPAREN:
      return new Alt(readOptions<Altn>(&Parser::readAltn));
    
    case Lexer::t_LSQ:
      return new RepAlt(readRep(), readAltn());
    }

  // case = "case" <expr> "{" {0 "|" <selection> } "}"
  //      | "alt" <rep> <selection>
  case Lexer::t_CASE:
    // TODO
    return NULL;

  // loop = "while" <expr> "do" <cmd>
  case Lexer::t_WHILE:
    // TODO
    return NULL;

  // loop = "do" <cmd> "while" <expr>
  case Lexer::t_DO:
    // TODO
    return NULL;

  // loop = "until" <expr> "do" <cmd>
  case Lexer::t_UNTIL:
    // TODO
    return NULL;

  // Declaration or abbreviation
  case Lexer::t_VAL: 
  case Lexer::t_VAR:
  case Lexer::t_CHAN: 
  case Lexer::t_CALL: 
  case Lexer::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lexer::t_COLON, "':' missing");
    return new CmdSpec(spec, readCmd());
  }

  // Disallowed
  case Lexer::t_PROC:
  case Lexer::t_SERV:
  case Lexer::t_FUNC:
    error("definition in specification of command");
    readSpec();
    return new CmdSpec(NULL, readCmd());
  }
}

// choice = <cond>
//        | <spec> ":" <choice>
//        | <expr> ":" <cmd>
Choice *Parser::readChoice() {
  switch(curTok) {
  default: break;
  
  // Nested test
  case Lexer::t_TEST:
    return new NestedChoice((Test*) readCmd());

  // Choice specification
  case Lexer::t_VAL:
  case Lexer::t_VAR: 
  case Lexer::t_CHAN: 
  case Lexer::t_CALL: 
  case Lexer::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lexer::t_COLON, "':' missing");
    return new SpecChoice(spec, readChoice());
  }
  
  // Disallowed specifications
  case Lexer::t_PROC:
  case Lexer::t_SERV:
  case Lexer::t_FUNC:
    error("definition in specification of choice");
    readSpec();
    return new SpecChoice(NULL, readChoice());
  }

  // Choice (assume <expr>)
  Expr *expr = readExpr();
  checkFor(Lexer::t_COLON, "':' missing");
  return new GuardedChoice(expr, readCmd());
}

// altn  = <alt>
//       | <spec> ":" <altn>
//       | <guard> ":" <cmd>
// guard = <elem> "?" <elem>
//       | <expr> "&" <elem> "?" <elem>
//       | <expr> "&" <skip>
Altn *Parser::readAltn() {
  switch(curTok) {
  default: break;
  
  // Nested alt
  case Lexer::t_ALT:
    return new NestedAltn((Alt*) readCmd());

  // Alternative specification
  case Lexer::t_VAL:
  case Lexer::t_VAR: 
  case Lexer::t_CHAN: 
  case Lexer::t_CALL: 
  case Lexer::t_INTF: {
    Spec *spec = readSpec();
    checkFor(Lexer::t_COLON, "':' missing");
    return new SpecAltn(spec, readAltn());
  }
  
  // Disallowed specifications
  case Lexer::t_PROC:
  case Lexer::t_SERV:
  case Lexer::t_FUNC:
    error("definition in specification of alternative");
    readSpec();
    return new SpecAltn(NULL, readAltn());
  }

  // Alternative
  // TODO: unguarded, guarded, skip
  //Expr *expr = readExpr();
  //checkFor(Lexer::t_COLON, "':' missing");
  //return new Altn(expr, elem, elem, readCmd());
  return NULL;
}

// rep = "[" {1 "," <index-range> "]"
Rep *Parser::readRep() {
  checkFor(Lexer::t_LSQ, "'[' missing");
  std::list<IndexRange*> *ranges = new std::list<IndexRange*>();
  do {
    IndexRange *range = readIndexRange();
    ranges->push_back(range);
  } while (curTok == Lexer::t_COMMA);
  checkFor(Lexer::t_RSQ, "']' missing");
  return new Rep(ranges);
}

// index-range = <name> "=" <expr> "for" <expr>
//             | <name> "=" <expr> "for" <expr> "step" <expr>
IndexRange *Parser::readIndexRange() {
  Name *name = readName();
  checkFor(Lexer::t_EQ, "'=' missing");
  Expr *base = readExpr();
  checkFor(Lexer::t_FOR, "'for' missing");
  Expr *count = readExpr();
  Expr *step = NULL;
  if (curTok == Lexer::t_STEP) {
    getNextToken();
    step = readExpr();
  }
  return new IndexRange(name, base, count, step);
}

// "|" separated list, "{" {0 "|" <item> } "}"
// item = choice | altn | selection
template<typename T>
std::list<T*> *Parser::readOptions(T *(Parser::*readItem)()) {
  checkFor(Lexer::t_LPAREN, "'{' missing");
  std::list<T*> *l = new std::list<T*>();
  do {
    l->push_back((this->*readItem)());
    getNextToken();
  } while (curTok != Lexer::t_OR);
  checkFor(Lexer::t_RPAREN, "'}' missing");
  return l;
}

// "," separated list, "(" {0 "," <item> } ")"
// item = fml | decl | expr
template<typename T>
std::list<T*> *Parser::readList(T *(Parser::*readItem)()) {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<T*> *l = new std::list<T*>();
  do {
    l->push_back((this->*readItem)());
    getNextToken();
  } while (curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return l;
}

// elem = ...
Elem *Parser::readElem() {
  return NULL;
}

// <name>
Name *Parser::readName() {
  std::string *s = &LEX.s;
  checkFor(Lexer::t_NAME, "name expected");
  return new Name(*s);
}

// expr = ...
Expr *Parser::readExpr() {
  return NULL;
}

// {0 "," <name> }
std::list<Name*> *Parser::readNames() {
  std::list<Name*> *names = new std::list<Name*>();
  do {
    getNextToken();
    // In formal lists, don't read next specifier
    if (curTok != Lexer::t_NAME)
      break;
    names->push_back(readName());
  } while (curTok != Lexer::t_COMMA);
  return names;
}

