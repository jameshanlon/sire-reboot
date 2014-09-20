#include "Tree.h"
#include <stdio.h>
#include <stdarg.h>

// Indent
static bool indent[100];
static void ind(int x) {
  //printf("%*.*s", x, x, "");
  indent[x-1] = true;
  for (int i=0; i<x; ++i)
    printf("%s", indent[i] ? "| " : "  ");
  printf("*-");
}
static void und(int x) {
  indent[x-1] = false;
}

void Tree::print() {
  for (auto x : spec) printSpec(0, x);
  for (auto x : prog) printCmd(0, x);
}

void Tree::printSpec(int x, Spec *s) {
  switch(s->type) {
  case Spec::DEF:   printDef(x, (Def*) s);   break;
  case Spec::DECL:  printDecl(x, (Decl*) s); break;
  case Spec::ABBR:  printAbbr(x, (Abbr*) s); break;
  case Spec::SSPEC:
    ind(x); printf("SimultaenousSpecification");
    for (auto y : *(((SimSpec*) s)->specs))
      printSpec(x+1, y);
    break;
  }
}

void Tree::printDef(int x, Def *d) {
  ind(x);
  switch(d->type) {
  
  case Def::PROCESS:
    printf("ProcessDef (%s)\n", d->name->cstr());
    printFmls(x+1, d->args);
    printProcess(x+1, ((ProcessDef*) d)->process);
    break;
  
  case Def::SERVER:   
    printf("ServerDef (%s)\n", d->name->cstr());
    printFmls(x+1, d->args);
    printServer(x+1, ((ServerDef*) d)->server);
    break;
  
  case Def::ISERVER:  
    printf("InheritingServerDef (%s)\n", d->name->cstr());
    printFmls(x+1, d->args);
    printHiding(x+1, ((InhrtServerDef*) d)->hiding);
    break;
  
  case Def::FUNCTION: 
    printf("FunctionDef (%s)\n", d->name->cstr());
    printFmls(x+1, d->args);
    printExpr(x+1, ((FunctionDef*) d)->expr);
    break;
  }
  und(x);
}

void Tree::printDecl(int x, Decl *d) {
  switch(d->tDecl) {
  case Decl::VAR:     break;
  case Decl::ARRAY:   break;
  case Decl::HIDING:  break;
  case Decl::SERVER:  break;
  case Decl::RSERVER: break;
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
  ind(x); printf("Formals\n");
  if (f != nullptr) {
    for (auto y : *f)
      printFml(x+1, y);
  }
}

void Tree::printFml(int x, Fml *f) {
  ind(x); printf("Formal\n");
}

void Tree::printProcess(int x, Process *p) {
  ind(x); printf("Process\n");
}

void Tree::printServer(int x, Server *s) {
  ind(x); printf("Server\n");
}

void Tree::printHiding(int x, Hiding *h) {
  ind(x); printf("Hiding\n");
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

void Tree::printExpr(int x, Expr *e) {
  ind(x); printf("expr\n");
}

