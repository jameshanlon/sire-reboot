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
  LEX.readToken();
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
     || curTok == Lexer::t_PROCESS 
     || curTok == Lexer::t_SERVER 
     || curTok == Lexer::t_FUNCTION) {
    Spec *spec = readSpec();
    
    // Read a sequential specification
    switch(curTok) {
    default:
      error("invalid separator for specification");
      break;
    case Lexer::t_COLON:
      tree->spec.push_back(readSpec());
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
  default:                assert(0 && "invalid token");
  case Lexer::t_EOF:      return NULL;
  case Lexer::t_VAL:      return readValAbbr();
  case Lexer::t_VAR:      return readDeclAbbr();
  case Lexer::t_PROCESS:  return readProc();
  case Lexer::t_SERVER:   return readServ();
  case Lexer::t_FUNCTION: return readFunc();
  }
}

// definition = "process" ...
//            | "server" ...
//            | "function" ...
Def *Parser::readDef() {
  switch(curTok) {
  default:                error("expecting definition");
  case Lexer::t_PROCESS:  return readProc();
  case Lexer::t_SERVER:   return readServ();
  case Lexer::t_FUNCTION: return readFunc();
  }
}

// abbreviation = <specifier> <name> "is" <element>
//              | "call" <name> "(" {0 "," <formal> } ")" "is" <name>
Abbr *Parser::readValAbbr() {
  getNextToken();
  Spef *s = readSpef();
  Name *n = readName();
  return new VarAbbr(s, n, readElem()); 
}

// specification = <declaration>
//               | <abbreviation>
// declaration   = <specifier> {1 "," <name> }
// abbreviation  = <specifier> <name> "is" <element>
Spec *Parser::readDeclAbbr() {
  Spef *s = readSpef();
  Name *n = readName();
  Spec *res;
  switch (curTok) {
  default: assert(0 && "invalid token");
  
  // Single-name declaration
  case Lexer::t_COLON:
    res = (Spec *) new VarDecl(s, n);
    break;

  // Name list declaration
  case Lexer::t_COMMA:
    res = (Spec *) new VarDecl(s, readNames());
    break;
  
  // Abbreviation
  case Lexer::t_IS:
    res = (Spec *) new VarAbbr(s, n, readElem());
    break;
  }

  checkFor(Lexer::t_COLON, "expected ':' after declaration or abbreviation");
  return res;
}

// specifier = <type>
//           | <specifier> "[" "]"
//           | <specifier> "[" <expr> "]"
// type      = "var"
//           | "chanend"
Spef *Parser::readSpef() {
  switch (curTok) {
  default: error("invalid specifier");
  
  case Lexer::t_VAR:
    getNextToken();

    // Variable specifier
    if (curTok == Lexer::t_NAME)
      return (Spef *) new VarSpef();

    // Array specififer
    std::list<Expr*> *lengths = new std::list<Expr*>();
    while (curTok == Lexer::t_LSQUARE) {
      lengths->push_back(readExpr());
      checkFor(Lexer::t_RSQUARE, "']' missing");
      getNextToken();
    }
    return (Spef *) new ArraySpef(lengths);
  }
}

// def = process <name> "(" {0 "," <formal>} ")" "is" <cmd>
Def *Parser::readProc() {
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readFmls();
  checkFor(Lexer::t_RPAREN, "')' missing");

  checkFor(Lexer::t_IS, "'is' missing");
  if(curTok == Lexer::t_INTERFACE) {
    std::list<Decl*> *interfaces = readInterfaces();
    checkFor(Lexer::t_TO, "'to' missing");
  }
  Cmd *cmd = readCmd();
  return new Def(Def::PROCESS, name, args, cmd);
}

// definition  = "server" <name> "(" {0, <formal>} ")" "inherits" <hiding-decl>
//             | "server" <name> "(" {0, <formal>} ")" "is" <cmd>
// hiding-decl = "from" { {1 : <declaration> } } "interface" <name>
Def *Parser::readServ() {
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *args = readFmls();
  checkFor(Lexer::t_RPAREN, "')' missing");

  // Inheriting definition
  if(curTok == Lexer::t_INHERITS) {
    // return ...
  }

  checkFor(Lexer::t_IS, "'is' missing");
  if(curTok == Lexer::t_INTERFACE) {
    std::list<Decl*> *interfaces = readInterfaces();
    checkFor(Lexer::t_TO, "'to' missing");
  }
  Cmd *cmd = readCmd();
  return new Def(Def::PROCESS, name, args, cmd);
}

// def = function <name> ( {0, <formal>} ) is <valof>
//     | function <name> ( {0, <formal>} ) is <expr>
Def *Parser::readFunc() {
  return (Def*) NULL;
}


// <declaration>
// declaration = "call" {1 "," <name> "(" <formal> ")" }
Decl *Parser::readInterface() {
  return NULL;
}

// formal = <specifier> {1 "," <name> }
//        | "val" <specifier> {1 "," <name> }
//        | "call" <name> {1 "," <name> "(" <formal> ")" }
Fml *Parser::readFml() {
  return NULL;
}

// cmd = ...
Cmd *Parser::readCmd() {
  switch(curTok) {
  default: LEX.error("bad outer-level declaration or command");
  }
  return (Cmd *) NULL;
}

// <name>
Name *Parser::readName() {
  checkFor(Lexer::t_NAME, "name expected");
  return new Name(LEX.s);
}

// "(" {0 "," <formal> } ")"
std::list<Fml*> *Parser::readFmls() {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Fml*> *fmls = new std::list<Fml*>();
  do {
    fmls->push_back(readFml());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return fmls;
}

// "(" {0 "," <declaration>} ")";
std::list<Decl*> *Parser::readInterfaces() {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::list<Decl*> *interfaces = new std::list<Decl*>();
  do {
    interfaces->push_back(readInterface());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
  return interfaces;
}
// {0 "," <name> }
std::list<Name*> *Parser::readNames() {
  std::list<Name*> *names = new std::list<Name*>();
  do {
    getNextToken();
    names->push_back(readName());
  } while(curTok != Lexer::t_COMMA);
  return names;
}

