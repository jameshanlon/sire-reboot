#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include <map>

#define TAB SymTable::get()

class SymTable {
public:
  static SymTable instance;
  static SymTable &get() { return instance; }
  SymTable() {}
  ~SymTable() {}
  void init();
  void insert(std::string name, int t);
  int lookup(std::string name);
  
private:
  std::map<std::string, int> table;
};

#endif
