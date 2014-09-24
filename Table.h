#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include "Lex.h"

#include <string>
#include <map>

#define TAB Table::get()

class Table {
public:
  static Table instance;
  static Table &get() { return instance; }
  Table() {}
  ~Table() {}
  void init();
  void insert(const std::string &name, const Lex::Token t);
  int lookup(const std::string &name);

private:
  std::map<std::string, Lex::Token> table;
};

#endif
