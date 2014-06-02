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

// program       = <specification> ":" <program>
//               | <sequence>;
// specification = <declaration>
//               | <abbreviation>
//               | <definition>
// definition    = {0 "&" <definition>}
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

// specification = <declaration>
//               | <abbreviation>
//               | <definition>
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

// abbreviation = <specifier> <name> "is" <element>
//              | "call" <name> "(" {0 "," <formal> } ")" "is" <name>
Abbr *Parser::readValAbbr() {
  getNextToken();
  Spef *s = readSpef(true);
  Name *n = readName();
  return new VarAbbr(s, n, readElem()); 
}

// specification = <declaration>
//               | <abbreviation>
// declaration   = <specifier> {1 "," <name> }
// abbreviation  = <specifier> <name> "is" <element>
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

// specifier = <type>
//           | <type> <name>
//           | <specifier> "[" "]"
//           | <specifier> "[" <expr> "]"
// type      = "var"
//           | "chanend"
//           | "call"
//           | "interface" "(" {0 "," <decl> } ")"
//           | "process"
//           | "server"
//           | "function"
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
    return new IntfSpef(Spef::INTF, readIntfs(), readDims());

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

// def = process <name> "(" {0 "," <formal>} ")" "is" <cmd>
Def *Parser::readProc() {
  assert(curTok == Lexer::t_PROC);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readFmls();
  checkFor(Lexer::t_RPAREN, "')' missing");
  checkFor(Lexer::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lexer::t_INTF) {
    intfs = readIntfs();
    checkFor(Lexer::t_TO, "'to' missing");
  }
  return new Proc(name, args, intfs, readCmd());
}

// definition  = "server" <name> "(" {0, <formal>} ")" "inherits" <hiding-decl>
//             | "server" <name> "(" {0, <formal>} ")" "is" <cmd>
// hiding-decl = "from" { {1 : <declaration> } } "interface" <name>
Def *Parser::readServ() {
  assert(curTok == Lexer::t_SERV);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readFmls();
  checkFor(Lexer::t_RPAREN, "')' missing");

  // Inheriting definition
  if(curTok == Lexer::t_INHRT) {
    // TODO
    // return ...
  }

  checkFor(Lexer::t_IS, "'is' missing");
  std::list<Decl*> *intfs = NULL;
  if(curTok == Lexer::t_INTF) {
    intfs = readIntfs();
    checkFor(Lexer::t_TO, "'to' missing");
  }
  // read server decls
  return new Serv(name, args, intfs, NULL);
}

// def = "function" <name> "(" {0 "," <formal>} ")" "is" <expr>
Def *Parser::readFunc() {
  assert(curTok == Lexer::t_FUNC);
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readFmls();
  checkFor(Lexer::t_RPAREN, "')' missing");
  checkFor(Lexer::t_IS, "'is' missing");
  return new Func(name, args, readExpr());
}

// <declaration>
// declaration = "call" {1 "," <name> "(" <formal> ")" }
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
      std::list<Fml*> *args = readFmls();
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
          argss->push_back(readFmls());
        }
        return new CallDecl(spef, names, argss);
      }
    }
  }
}

// formal = <specifier> {1 "," <name> }
//        | "val" <specifier> {1 "," <name> }
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
//            | "seq" <replicator> ":" <cmd>
//            | <specification> ":" <cmd>
// prim-cmd   = <assignment>
//            | <connect> 
//            | <input>
//            | <output>
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
      std::list<Expr*> *actuals = readActuals();
      checkFor(Lexer::t_RPAREN, "')' missing");
      return new Instance(name, actuals);
    }
    
    // Call
    case Lexer::t_DOT: {
        getNextToken();
        Name *field = readName();
        checkFor(Lexer::t_LPAREN, "'(' missing");
        std::list<Expr*> *actuals = readActuals();
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
        return IfD(expr, readCmd());
      
      case Lexer::t_THEN: {
        Cmd *thenCmd = readCmd();
        checkFor(Lexer::t_ELSE, "'else' missing");
        return IfTE(expr, thenCmd, readCmd());
    }
    return NULL;
  }

  // cond = "test" "{" {0 "|" <choice> } "}" 
  case Lexer::t_TEST: {
    getNextToken();
    checkFor(Lexer::t_LPAREN, "'{' missing");
    std::list<Choice*> *choices = readChoices();
    checkFor(Lexer::t_RPAREN, "'}' missing");
    return Test(choices);
  }

  case Lexer::t_ALT:
    return NULL;

  case Lexer::t_CASE:
    return NULL;

  case Lexer::t_WHILE:
    return NULL;

  case Lexer::t_DO:
    return NULL;

  case Lexer::t_UNTIL:
    return NULL;

  // Declaration or abbreviation
  case Lexer::t_VAL: 
  case Lexer::t_VAR: {
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

// "(" {0 "," <formal> } ")"
std::list<Fml*> *Parser::readFmls() {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *fmls = new std::list<Fml*>();
  do {
    fmls->push_back(readFml());
    getNextToken();
  } while (curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return fmls;
}

// "(" {0 "," <expr> } ")"
std::list<Expr*> *Parser::readActuals() {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Expr*> *actuals = new std::list<Expr*>();
  do {
    actuals->push_back(readExpr());
    getNextToken();
  } while (curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return actuals;
}

// "(" {0 "," <decl>} ")";
std::list<Decl*> *Parser::readIntfs() {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Decl*> *interfaces = new std::list<Decl*>();
  do {
    interfaces->push_back(readIntf());
    getNextToken();
  } while (curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return interfaces;
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

