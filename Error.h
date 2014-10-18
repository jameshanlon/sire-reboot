#ifndef ERROR_H
#define ERROR_H

#include "Lex.h"

#include <string>
#include <exception>

#define MAX_ERRORS 8
#define ERR Error::get()

class FatalError {
  std::string s;
public:
  FatalError() : s(nullptr) {}
  FatalError(const char *s) : s() {}
  ~FatalError() throw() {}
  bool hasMsg() { return s.empty(); }
  const char* msg() const throw() {
    return s.c_str();
  }
};

class Error {
public:
  Error() : count(0) {};
  ~Error() {};
  static Error instance;
  static Error &get() { return instance; }
  bool any() { return count > 0; }
  void record() {
    count++;
    if(count >= MAX_ERRORS)
      throw FatalError("too many errors");
  }
private:
  int count;
};

#endif
