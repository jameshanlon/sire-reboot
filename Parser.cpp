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
    if (spec->type == Lexer::t_COLON)
      tree->spec.push_back(readSpec());
    // Read simultaneous definitions
    else if (curTok == Lexer::t_AND) {
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
    }
    else
      error("invalid separator for specification");
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
  case Lexer::t_VAR:      return readDecl();
  case Lexer::t_VAL:      return readDecl();
  case Lexer::t_FUNCTION: return readFunc();
  case Lexer::t_PROCESS:  return readProc();
  case Lexer::t_SERVER:   return readServ();
  }
}

Decl *Parser::readDecl() {
  Spef *s = readSpef();
  Name *n = readName();
  switch (curTok) {
  default:
  // Abbreviation
  case Lexer::t_IS:
    return VarAbbr(s, n, readElem());  
  // Name list
  case Lexer::t_COMMA: {
    std::vector<Name*> names = new std::vector<Name*>();
    readNames(names);
    return 
    }
  // End
  case Lexer::t_COLON:
  }
  return (Decl *) NULL;
}

// specifier      = <primitive-type>
//                | <specifier> [ ]
//                | <specifier> [ <expr> ]
// primitive-type = var
Spef *Parser::readSpef() {
  switch (curTok) {
  default: error("invalid specifier");
  
  // Value specifier
  case Lexer::t_VAL:
    return ValSpef();

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

// def = process <name> ( {0, <formal>} ) inherits ...
//     | process <name> ( {0, <formal>} ) is <cmd>
Def *Parser::readProc() {
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

// def := <name> ( {0, <formal>} ) is ...
Def *Parser::readServ() {
  return NULL;
}

// def := function <name> ( {0, <formal>} ) is <valof>
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

