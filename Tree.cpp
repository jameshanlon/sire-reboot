#include "Tree.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// Indenting with tree markers
static int markers[100] = {1};
static void indent(int i, int numChildren) {
  markers[i] = numChildren;
  if(i > 1) --markers[i-1];
  for (int j=0; j<i-1; ++j)
    printf("%s", markers[j] > 0 ? "| " : "  ");
  printf("*-");
}

void Tree::print() {
  for (auto x : spec) printSpec(1, x);
  for (auto x : prog) printCmd(1, x);
}

void Tree::printSpec(int i, Spec *s) {
  switch(s->type) {
  case Spec::DEF:   printDef(i, (Def*) s);   break;
  case Spec::DECL:  printDecl(i, (Decl*) s); break;
  case Spec::ABBR:  printAbbr(i, (Abbr*) s); break;
  case Spec::SSPEC: {
      SimSpec *sim = (SimSpec*) s;
      indent(i, sim->specs->size());
      printf("SimultaenousSpecification");
      for (auto y : *(sim->specs))
        printSpec(i+1, y);
      break;
    }
  }
}

void Tree::printDef(int i, Def *d) {
  switch(d->type) {

  case Def::PROCESS: {
      indent(i, 3);
      ProcessDef *x = static_cast<ProcessDef*>(d);
      printf("ProcessDef\n");
      printName(i+1, x->name);
      printFmls(i+1, x->args);
      printProcess(i+1, x->process);
      break;
    }

  case Def::SERVER: {
      indent(i, 3);
      ServerDef *x = static_cast<ServerDef*>(d);
      printf("ServerDef\n");
      printName(i+1, x->name);
      printFmls(i+1, x->args);
      printServer(i+1, x->server);
      break;
    }

  case Def::ISERVER: {
      indent(i, 3);
      InhrtServerDef *x = static_cast<InhrtServerDef*>(d);
      printf("InheritingServerDef\n");
      printName(i+1, x->name);
      printFmls(i+1, x->args);
      printHiding(i+1, x->hiding);
      break;
    }

  case Def::FUNCTION: {
      indent(i, 3);
      FunctionDef *x = static_cast<FunctionDef*>(d);
      printf("FunctionDef\n");
      printName(i+1, x->name);
      printFmls(i+1, x->args);
      printExpr(i+1, x->expr);
      break;
    }
  }
}

void Tree::printDecl(int i, Decl *d) {
  switch(d->tDecl) {

  case Decl::VAR:
    indent(i, 0);
    printf("Decl var\n");
    break;

  case Decl::ARRAY:
  case Decl::HIDING:
  case Decl::SERVER:
  case Decl::RSERVER:
    indent(i, 0);
    printf("Decl todo\n");
  }
}

void Tree::printAbbr(int i, Abbr *a) {
  switch(a->type) {
  case Abbr::VAR:
  case Abbr::CALL:
  case Abbr::SERVER:
  case Abbr::PROCESS:
  case Abbr::FUNCTION:
    indent(i, 0);
    printf("Abbr todo\n");
    break;
  }
}

void Tree::printFmls(int i, std::list<Fml*> *f) {
  indent(i, f->size());
  printf("Formals\n");
  if (f != nullptr) {
    for (auto y : *f)
      printFml(i+1, y);
  }
}

void Tree::printFml(int i, Fml *f) {
  indent(i, 1);
  printf("Formal\n");
  printName(i+1, f->name);
}

void Tree::printProcess(int i, Process *p) {
  switch (p->type) {
  default: assert(0 && "invalid process type");

  case Process::CMD: {
      ProcessCmd *x = static_cast<ProcessCmd*>(p);
      indent(i, 1);
      printf("Process\n");
      printCmd(i+1, x->cmd);
      break;
    }

  case Process::SPEC: {
      ProcessSpec *x = static_cast<ProcessSpec*>(p);
      indent(i, 2);
      printf("Process\n");
      printIntf(i+1, x->intf);
      printCmd(i+1, x->cmd);
      break;
    }
  }
}

void Tree::printServer(int i, Server *s) {
  indent(i, 0);
  printf("Server\n");
}

void Tree::printIntf(int i, std::list<Decl*> *f) {
  indent(i, 0);
  printf("Interface\n");
}

void Tree::printHiding(int i, Hiding *h) {
  indent(i, 0);
  printf("Hiding\n");
}

void Tree::printCmd(int i, Cmd *c) {
  switch(c->type) {

  case Cmd::SPEC: {
      CmdSpec *x = static_cast<CmdSpec*>(c);
      printSpec(i, x->spec);
      printCmd(i+1, x->cmd);
      break;
    }

  case Cmd::SKIP:
    indent(i, 0);
    printf("Skip\n");
    break;

  case Cmd::STOP:
    indent(i, 0);
    printf("Stop\n");
    break;

  case Cmd::SEQ: {
      Seq *x = static_cast<Seq*>(c);
      indent(i, x->cmds->size());
      printf("Seq\n");
      for (auto y : *x->cmds)
        printCmd(i+1, y);
      break;
    }

  case Cmd::ASS: {
      indent(i, 0);
      Ass *x = static_cast<Ass*>(c);
      break;
    }

  case Cmd::IN: {
      indent(i, 0);
      In *x = static_cast<In*>(c);
      break;
    }

  case Cmd::OUT: {
      indent(i, 0);
      Out *x = static_cast<Out*>(c);
      break;
    }

  case Cmd::CONNECT: {
      indent(i, 0);
      Connect *x = static_cast<Connect*>(c);
      break;
    }

  case Cmd::ALT: {
      indent(i, 0);
      Alt *x = static_cast<Alt*>(c);
      break;
    }

  case Cmd::TEST: {
      indent(i, 0);
      Test *x = static_cast<Test*>(c);
      break;
    }

  case Cmd::IFD: {
      indent(i, 0);
      IfD *x = static_cast<IfD*>(c);
      break;
    }

  case Cmd::IFTE: {
      indent(i, 0);
      IfTE *x = static_cast<IfTE*>(c);
      break;
    }

  case Cmd::CASE: {
      indent(i, 0);
      Case *x = static_cast<Case*>(c);
      break;
    }

  case Cmd::WHILE: {
      indent(i, 0);
      While *x = static_cast<While*>(c);
      break;
    }

  case Cmd::UNTIL: {
      indent(i, 0);
      Until *x = static_cast<Until*>(c);
      break;
    }

  case Cmd::DO: {
      indent(i, 0);
      Do *x = static_cast<Do*>(c);
      break;
    }

  case Cmd::PAR: {
      indent(i, 0);
      Par *x = static_cast<Par*>(c);
      break;
    }

  case Cmd::INSTANCE: {
      indent(i, 0);
      Instance *x = static_cast<Instance*>(c);
      break;
    }

  case Cmd::CALL: {
      indent(i, 0);
      Call *x = static_cast<Call*>(c);
      break;
    }

  case Cmd::RALT: {
      indent(i, 0);
      RepAlt *x = static_cast<RepAlt*>(c);
      break;
    }

  case Cmd::RTEST: {
      indent(i, 0);
      RepTest *x = static_cast<RepTest*>(c);
      break;
    }

  case Cmd::RCASE: {
      indent(i, 0);
      RepCase *x = static_cast<RepCase*>(c);
      break;
    }

  case Cmd::RSEQ: {
      indent(i, 0);
      RepSeq *x = static_cast<RepSeq*>(c);
      break;
    }
  }
}

void Tree::printExpr(int i, Expr *e) {
  indent(i, 0);
  printf("Expr\n");
}

void Tree::printName(int i, Name *name) {
  indent(i, 0);
  printf("Name %s\n", name->str.c_str());
}

