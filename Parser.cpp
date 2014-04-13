#include "Parser.h"
#include "Error.h"

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

// Outer-lever declarations
Tree *Parser::readProgram() {
  switch(curTok) {
  default: LEX.error("bad outer-level declaration");
  case Lexer::t_EOF: break;
  case Lexer::t_VAL: break;
  case Lexer::t_VAR: break;
  case Lexer::t_PROCESS: break;//return Tree(token, element, readProgram);
  case Lexer::t_SERVER:  break;
  }
  return new Tree(); 
}

