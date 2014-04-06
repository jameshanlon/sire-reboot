#include "Parser.h"

Lexer::Token getNextToken() {
  return curTok = Lexer::getToken();
}

//void parse() {
//  unsigned char c;  
//  do {
//    c = getchar();
//  } while (c != '\n');
//}


