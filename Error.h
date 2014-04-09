#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>

#include "Lexer.h"

class FatalError {
  std::string s;
public:
  FatalError(const char *s) : s(s) {}
  ~FatalError() throw() {}
  const char* msg() const throw() {
    return s.c_str();
  }
};

void fatalErr(const char *msg) {
  throw FatalError(msg); 
}

void synErr() {
  printf("Error near line %d: %s", lineNum);
  lex().printChBuf();
  // FatalError if too many errors
}

#endif
