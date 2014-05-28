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

// program       = <specification> : <program>
//               | <sequence>;
// specification = <declaration>
//               | <abbreviation>
//               | <definition>
// definition    = {0 & <definition>}
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
      std::vector<Def*> defs = new std::vector<Def*>();
      defs.push_back(spec);
      do {
        getNextToken();
        defs.push_back(readDef());
      } while (curTok == Lexer::AND);
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
  case Lexer::t_VAR:      return readDecl();
  case Lexer::t_FUNCTION: return readFunc();
  case Lexer::t_PROCESS:  return readProc();
  case Lexer::t_SERVER:   return readServ();
  }
}

// abbreviation = <specifier> <name> is <element>
Decl *Parser::readValAbbr() {
  getNextToken();
  Spef *s = readSpef();
  Name *n = readName();
  return VarAbbr(s, n, readElem());  
}

// declaration  = <specifier> {1 , <name> }
// abbreviation = <specifier> <name> is <element>
Decl *Parser::readDecl() {
  Spef *s = readSpef();
  Name *n = readName();
  switch (curTok) {
  default: assert(0 && "invalid token");
  
  // Name list
  case Lexer::t_COMMA: {
    std::vector<Name*> names = new std::vector<Name*>();
    readNames(names);
    return VarDecl(names);
    }
  
  // Abbreviation
  case Lexer::t_IS:
    return VarAbbr(s, n, readElem());  
  }

  checkFor(Lexer::t_COLON, "expected ':' after declaration or abbreviation");
  return (Decl *) NULL;
}

// specifier = <type>
//           | <specifier> [ ]
//           | <specifier> [ <expr> ]
// type      = var
Spef *Parser::readSpef() {
  switch (curTok) {
  default: error("invalid specifier");
  
  case Lexer::t_VAR:
    getNextToken();

    // Variable specifier
    if (curTok == Lexer::t_NAME)
      return VarSpef();

    // Array specififer
    std::vector<Expr*> lengths = new std::vector<Expr*>();
    while (curTok == Lexer::t_LSQUARE) {
      lengths.push_pack(readExpr());
      checkFor(Lexer::t_RSQUARE, "']' missing");
      getNextToken();
    }
    return ArraySpef(lengths);
  }
}

// def = process <name> ( {0, <formal>} ) is <cmd>
Def *Parser::readProc() {
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::vector<Fml*> *args = new std::vector<Fml*>();
  readFmls(*args);
  checkFor(Lexer::t_RPAREN, "')' missing");

  checkFor(Lexer::t_IS, "'is' missing");
  if(curTok == Lexer::t_INTERFACE) {
    std::vector<Decl*> *interfaces = new std::vector<Decl*>();
    readInterfaces(*interfaces);
    checkFor(Lexer::t_TO, "'to' missing");
  }
  Cmd *cmd = readCmd();
  return new Def(Def::PROCESS, name, args, cmd);
}

// definition  = server <name> ( {0, <formal>} ) inherits <hiding-decl>
//             | server <name> ( {0, <formal>} ) is <cmd>
// hiding-decl = from { {1 : <declaration> } } interface <name>
Def *Parser::readServ() {
  getNextToken();
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::vector<Fml*> *args = new std::vector<Fml*>();
  readFmls(*args);
  checkFor(Lexer::t_RPAREN, "')' missing");

  // Inheriting definition
  if(curTok == Lexer::t_INHERITS) {
    // return ...
  }

  checkFor(Lexer::t_IS, "'is' missing");
  if(curTok == Lexer::t_INTERFACE) {
    std::vector<Decl*> *interfaces = new std::vector<Decl*>();
    readInterfaces(*interfaces);
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

void Parser::readFmls(std::vector<Fml*> &fmls) {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  do {
    fmls.push_back(readFml());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
}

void Parser::readInterfaces(std::vector<Decl*> &interfaces) {
  checkFor(Lexer::t_LPAREN, "'(' missing");
  do {
    interfaces.push_back(readInterface());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
  checkFor(Lexer::t_RPAREN, "')' missing");
}

// formal = ...
Fml *Parser::readFml() {
  return NULL;
}

// interface-decl = ...
Decl *Parser::readInterface() {
  return NULL;
}

// cmd = ...
Cmd *Parser::readCmd() {
  switch(curTok) {
  default: LEX.error("bad outer-level declaration or command");
  }
  return (Cmd *) NULL;
}

// name = ...
Name *Parser::readName() {
  checkFor(Lexer::t_NAME, "name expected");
  return new Name(LEX.s);
}

void Parser::readNames(std::vector<Name*> &names) {
  do {
    getNextToken();
    names.push_back(readName());
  } while(curTok != Lexer::t_COMMA);
}

