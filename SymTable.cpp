#include "SymTable.h"

SymTable SymTable::instance;

void SymTable::init() {
}

void SymTable::insert(std::string name, int t) {
  table.insert(std::make_pair(std::string(keyword), (int) t));
}

int SymTable::lookup(std::string name) {
  std::map<std::string, int>::const_iterator it = table.find(name);
  if(it != table.end())
    return it->second;
  table.insert(std::make_pair(s, t_NAME));
  return t_NAME;
}

 
