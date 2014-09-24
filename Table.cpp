#include "Table.h"

Table Table::instance;

void Table::init() {
}

void Table::insert(const std::string &name, Lex::Token t) {
  table.insert(std::make_pair(name, t));
}

int Table::lookup(const std::string &name) {
  std::map<std::string, Lex::Token>::const_iterator it = table.find(name);
  if(it != table.end())
    return it->second;
  table.insert(std::make_pair(name, Lex::tNAME));
  return Lex::tNAME;
}

