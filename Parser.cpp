#include "Parser.h"

Parser Parser::instance;

void Parser::getNextToken() {
  curTok = LEX.readToken();
}

//void parse() {
//  unsigned char c;  
//  do {
//    c = getchar();
//  } while (c != '\n');
//}


