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

Tree *Parser::formTree() {
  Tree *t = readProg();
  if(curTok != Lexer::t_EOF)
    throw FatalError("incorrect termination");
  return t;
}

Tree *Parser::readProg() {
  Tree *tree = new Tree();
  // Read specifications
  while(curTok == Lexer::t_VAL 
     || curTok == Lexer::t_VAR 
     || curTok == Lexer::t_PROCESS 
     || curTok == Lexer::t_SERVER 
     || curTok == Lexer::t_FUNCTION)
    tree->spec.push_back(readSpec());
  // Read program sequence
  while(curTok != Lexer::t_EOF)
    tree->prog.push_back(readCmd());
  return tree;
}

Spec *Parser::readSpec() {
  switch(curTok) {
  default:                assert(0 && "invalid token");
  case Lexer::t_EOF:      return NULL;
  case Lexer::t_VAR:      return readDecl();
  case Lexer::t_VAL:      return readAbbr();
  case Lexer::t_NAME:     return readAbbr();
  case Lexer::t_FUNCTION: return readFunc();
  case Lexer::t_PROCESS:  return readProc();
  case Lexer::t_SERVER:   return readServ();
  }
}

Decl *Parser::readDecl() {
  return (Decl *) NULL;
}

Decl *Parser::readAbbr() {
  return (Decl *) NULL;
}

Def *Parser::readProc() {
  Name *name = readName();
  checkFor(Lexer::t_LPAREN, "'(' missing");
  std::vector<Fml*> *args = new std::vector<Fml*>();
  readFmls(*args);
  checkFor(Lexer::t_RPAREN, "')' missing");
  checkFor(Lexer::t_IS, "'is' missing");
  if(curTok == Lexer::t_INHERITS) {
    // return ...
  }

  if(curTok == Lexer::t_INTERFACE) {
    std::vector<Decl*> *interfaces = new std::vector<Decl*>();
    readInterfaces(*interfaces);
    checkFor(Lexer::t_TO, "'to' missing");
  }
  Cmd *cmd = readCmd();
  return new Def(Def::PROCESS, name, args, cmd);
}

Def *Parser::readServ() {
  return NULL;
}

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

Fml *Parser::readFml() {
  return NULL;
}

Decl *Parser::readInterface() {
  return NULL;
}

Cmd *Parser::readCmd() {
  switch(curTok) {
  default: LEX.error("bad outer-level declaration or command");
  }
  return (Cmd *) NULL;
}

Name *Parser::readName() {
  checkFor(Lexer::t_NAME, "name expected");
  return new Name(LEX.s);
}

void Parser::readNames(std::vector<Name*> &names) {
  do {
    names.push_back(readName());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
}

