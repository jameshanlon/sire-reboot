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
  Tree *t = readProgram();
  if(curTok != Lexer::t_EOF)
    throw FatalError("incorrect termination");
  return t;
}

Tree *Parser::readProgram() {
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
  case Lexer::t_FUNCTION: return readFunction();
  case Lexer::t_PROCESS:  return readProcess();
  case Lexer::t_SERVER:   return readServer();
  }
}

Decl *Parser::readDecl() {
  return (Decl *) NULL;
}

Decl *Parser::readAbbr() {
  return (Decl *) NULL;
}

Def *Parser::readFunction() {
  return (Def*) NULL;
}

Def *Parser::readProcess() {
   Name *name = readName();
   checkFor(Lexer::t_LPAREN, "'(' missing");
   std::vector<Formal*> *args = readFmlList();
   checkFor(Lexer::t_LPAREN, "')' missing");
   if(curTok == Lexer::t_INHERITS) {
     // return ...
   }

   if(curTok == Lexer::t_INTERFACE) {
     // read interface
   }
   checkFor(Lexer::t_LPAREN, "':' missing");
   Cmd *cmd = readCmd();
   return new Def(Def::PROCESS, name, args, cmd);
}

Def *Parser::readServer() {
  return NULL;
}

std::vector<Formal*> *Parser::readFmlList() {
  std::vector<Formal*> *fmls = new std::vector<Formal*>();
  return fmls;
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

std::vector<Name*> *Parser::readNameList() {
  std::vector<Name*> *names = new std::vector<Name*>();
  do {
    names->push_back(readName());
    getNextToken();
  } while(curTok != Lexer::t_COMMA);
  return names;
}

