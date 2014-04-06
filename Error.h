#ifndef ERROR_H
#define ERROR_H

#include <string>
#include <exception>

class FatalError {
  std::string s;
public:
  FatalError(const char *s) : s(s) {}
  ~FatalError() throw() {}
  const char* msg() const throw() {
    return s.c_str();
  }
};

void fatalError(const char *msg) {
  throw FatalError(msg); 
}

void syntaxError() {
  printf("Error near line %d: %s", lineNum);
  // FatalError if too many errors
}

#endif
