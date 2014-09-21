#include "Tree.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// Indenting with tree markers
static int markers[100] = {1}; 
static void indent(int x, int numChildren) {
  markers[x] = numChildren;
  if(x > 1) --markers[x-1];
  for (int i=0; i<x-1; ++i)
    printf("%s", markers[i] > 0 ? "| " : "  ");
  printf("*-");
}

void Tree::print() {
  for (auto x : spec) printSpec(1, x);
  for (auto x : prog) printCmd(1, x);
}

void Tree::printSpec(int x, Spec *s) {
  switch(s->type) {
  case Spec::DEF:   printDef(x, (Def*) s);   break;
  case Spec::DECL:  printDecl(x, (Decl*) s); break;
  case Spec::ABBR:  printAbbr(x, (Abbr*) s); break;
  case Spec::SSPEC: {
      SimSpec *sim = (SimSpec*) s;
      indent(x, sim->specs->size()); 
      printf("SimultaenousSpecification");
      for (auto y : *(sim->specs))
        printSpec(x+1, y);
      break;
    }
  }
}

void Tree::printDef(int x, Def *d) {
  switch(d->type) {
  
  case Def::PROCESS:
    indent(x, 3);
    printf("ProcessDef\n");
    printName(x+1, d->name);
    printFmls(x+1, d->args);
    printProcess(x+1, ((ProcessDef*) d)->process);
    break;
  
  case Def::SERVER:   
    indent(x, 3);
    printf("ServerDef\n");
    printName(x+1, d->name);
    printFmls(x+1, d->args);
    printServer(x+1, ((ServerDef*) d)->server);
    break;
  
  case Def::ISERVER:  
    indent(x, 3);
    printf("InheritingServerDef\n");
    printName(x+1, d->name);
    printFmls(x+1, d->args);
    printHiding(x+1, ((InhrtServerDef*) d)->hiding);
    break;
  
  case Def::FUNCTION: 
    indent(x, 3);
    printf("FunctionDef\n");
    printName(x+1, d->name);
    printFmls(x+1, d->args);
    printExpr(x+1, ((FunctionDef*) d)->expr);
    break;
  }
}

void Tree::printDecl(int x, Decl *d) {
  switch(d->tDecl) {
  
  case Decl::VAR:
    indent(x, 0);
    printf("Decl var\n");
    break;
  
  case Decl::ARRAY:  
  case Decl::HIDING: 
  case Decl::SERVER: 
  case Decl::RSERVER:
    indent(x, 0);
    printf("Decl todo\n");
  }
}

void Tree::printAbbr(int x, Abbr *a) {
  switch(a->type) {
  case Abbr::VAR:      
  case Abbr::CALL:     
  case Abbr::SERVER:   
  case Abbr::PROCESS:  
  case Abbr::FUNCTION: 
    indent(x, 0);
    printf("Abbr todo\n");
    break;
  }
}

void Tree::printFmls(int x, std::list<Fml*> *f) {
  indent(x, f->size()); 
  printf("Formals\n");
  if (f != nullptr) {
    for (auto y : *f)
      printFml(x+1, y);
  }
}

void Tree::printFml(int x, Fml *f) {
  indent(x, 1); 
  printf("Formal\n");
  printName(x+1, f->name);
}

void Tree::printProcess(int x, Process *p) {
  switch (p->type) {
  default: assert(0 && "invalid process type");
  
  case Process::CMD:
    indent(x, 1); 
    printf("Process\n");
    printCmd(x+1, ((ProcessCmd*) p)->cmd);
    break;
  
  case Process::SPEC:
    indent(x, 2); 
    printf("Process\n");
    printIntf(x+1, ((ProcessSpec*) p)->intf);
    printCmd(x+1, ((ProcessSpec*) p)->cmd);
    break;
  }
}

void Tree::printServer(int x, Server *s) {
  indent(x, 0); 
  printf("Server\n");
}

void Tree::printIntf(int x, std::list<Decl*> *i) {
  indent(x, 0);
  printf("Interface\n");
}

void Tree::printHiding(int x, Hiding *h) {
  indent(x, 0); 
  printf("Hiding\n");
}

void Tree::printCmd(int x, Cmd *c) {
  switch(c->type) {

  case Cmd::SPEC:
    printSpec(x, ((CmdSpec*)c)->spec);
    printCmd(x+1, ((CmdSpec*)c)->cmd);
    break;
  
  case Cmd::SKIP:
    indent(x, 0);
    printf("Skip\n");
    break;
  
  case Cmd::STOP:
    indent(x, 0);
    printf("Stop\n");
    break;
  
  case Cmd::SEQ:
    indent(x, ((Seq*) c)->cmds->size());
    printf("Seq\n");
    for (auto y : *(((Seq*) c)->cmds))
      printCmd(x+1, y);
    break;
  
  case Cmd::ASS:     
  case Cmd::IN:      
  case Cmd::OUT:     
  case Cmd::CONNECT: 
  case Cmd::ALT:     
  case Cmd::TEST:    
  case Cmd::IFD:     
  case Cmd::IFTE:    
  case Cmd::CASE:    
  case Cmd::WHILE:   
  case Cmd::UNTIL:   
  case Cmd::DO:      
  case Cmd::PAR:     
  case Cmd::INSTANCE:
  case Cmd::CALL:    
  case Cmd::RALT:    
  case Cmd::RTEST:   
  case Cmd::RCASE:   
  case Cmd::RSEQ:
    indent(x, 0);
    printf("Cmd todo %d\n", c->type);
    break;
  }
}

void Tree::printExpr(int x, Expr *e) {
  indent(x, 0); 
  printf("Expr\n");
}

void Tree::printName(int x, Name *name) {
  indent(x, 0);
  printf("Name %s\n", name->str.c_str());
}

