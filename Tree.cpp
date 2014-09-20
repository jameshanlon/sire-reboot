#include "Tree.h"
#include <stdio.h>
#include <stdarg.h>

static const char *indent(int x) {
  char s[100];
  sprintf(s, "%*.*s", x, x, "");
  return s;
}

void Tree::print() {
  for (auto i : spec) printSpec(0, i);
  for (auto i : prog) printCmd(0, i);
}

void Tree::printSpec(int x, Spec *s) {
  switch(s->type) {
  case Spec::DEF:   printDef(x, (Def*) s);   break;
  case Spec::DECL:  printDecl(x, (Decl*) s); break;
  case Spec::ABBR:  printAbbr(x, (Abbr*) s); break;
  case Spec::SSPEC: break;
  }
}

void Tree::printDef(int x, Def *d) {
  switch(d->type) {
  case Def::PROCESS:
    printf("%sprocess-def (%s)\n", indent(x), d->name->str->c_str());
    printFmls(x+1, d->args);
    printProcess(x+1, ((ProcessDef*) d)->process);
    break;
  case Def::SERVER:   break;
  case Def::ISERVER:  break;
  case Def::FUNCTION: break;
  }
}

void Tree::printDecl(int x, Decl *d) {
  switch(d->type) {
  case Decl::VAR:   break;
  case Decl::ARRAY: break;
  case Decl::ABBR:  break;
  case Decl::SSPEC: break;
  }
}

void Tree::printAbbr(int x, Abbr *a) {
  switch(a->type) {
  case Abbr::VAR: break;
  case Abbr::CALL: break;
  case Abbr::SERVER: break;
  case Abbr::PROCESS: break;
  case Abbr::FUNCTION: break;
  }
}

void Tree::printFmls(int x, std::list<Fml*> *f) {
  printf("%sformals ()\n", indent(x));
}

void Tree::printProcess(int x, Process *p) {
  printf("%sprocess ()\n", indent(x));
}

void Tree::printCmd(int x, Cmd *c) {
  switch(c->type) {
  case Cmd::SPEC:     break;
  case Cmd::INSTANCE: break;
  case Cmd::CALL:     break;
  case Cmd::SKIP:     break;
  case Cmd::STOP:     break;
  case Cmd::ASS:      break;
  case Cmd::IN:       break;
  case Cmd::OUT:      break;
  case Cmd::CONNECT:  break;
  case Cmd::ALT:      break;
  case Cmd::TEST:     break;
  case Cmd::IFD:      break;
  case Cmd::IFTE:     break;
  case Cmd::CASE:     break;
  case Cmd::WHILE:    break;
  case Cmd::UNTIL:    break;
  case Cmd::DO:       break;
  case Cmd::SEQ:      break;
  case Cmd::PAR:      break;
  case Cmd::RALT:     break;
  case Cmd::RTEST:    break;
  case Cmd::RCASE:    break;
  case Cmd::RSEQ:     break;
  }
}

