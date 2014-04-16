#include "Tree.h"

void Tree::print() {
  // Specification
  for(std::vector<Node*>::iterator it=spec.begin(); it!=spec.end(); ++it) {
    printSpec(*it);
  }
  // Program
  for(std::vector<Node*>::iterator it=prog.begin(); it!=prog.end(); ++it) {
    printCmd(*it);
  }
}

void Tree::printSpec(Specification s) {
  switch(s.type) {
  case Spec::DEF:  printDef(s);  break;
  case Spec::DEC:  printDecl(s); break;
  case Spec::ABBR: printAbbr(s); break;
  }
}

void Tree::printDef(Def d) {
  switch(d.type) {
  case Def.SERVER:   break;
  case Def.PROCESS:  break;
  case Def.FUNCTION: break;
  }
}

void Tree::printDecl(Decl d) {
  switch(d.type) {
  case Decl::VAR:   break;
  case Decl::ARRAY: break;
  }
}

void Tree::printAbbr(Abbr a) {
  switch(d.type) {
  case Abbr::VAL: break;
  case Abbr::VAR: break;
  }
}

void Tree::printCmd(Cmd c) {
  switch(c.type) {
  case Cmd::SKIP:    break;
  case Cmd::STOP:    break;
  case Cmd::ASS:     break;
  case Cmd::IN:      break;
  case Cmd::OUT:     break;
  case Cmd::CONNECT: break;
  case Cmd::ALT:     break;
  case Cmd::COND:    break;
  case Cmd::LOOP:    break;
  case Cmd::SEQ:     break;
  case Cmd::PAR:     break;
  }
}

