#include "SymTable.h"

SymTable SymTable::instance;

void SymTable::init() {
}

void SymTable::insert(std::string name, Lexer::Token t) {
  table.insert(std::make_pair(std::string(name), t));
}

int SymTable::lookup(std::string name) {
  std::map<std::string, Lexer::Token>::const_iterator it = table.find(name);
  if(it != table.end())
    return it->second;
  table.insert(std::make_pair(name, Lexer::t_NAME));
  return Lexer::t_NAME;
}

 
