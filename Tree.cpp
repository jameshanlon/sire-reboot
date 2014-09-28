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
      SimSpec *x = static_cast<SimSpec*>(s);
      indent(i, x->specs->size());
      printf("SimultaenousSpecification");
      for (auto y : *x->specs)
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
      printHidingDecl(i+1, x->hidingDecl);
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

  case Decl::VAR: {
      indent(i, 0);
      VarDecl *x = static_cast<VarDecl*>(d);
      printf("VarDecl\n");
      break;
    }

  case Decl::HIDING: {
      indent(i, 0);
      HidingDecl *x = static_cast<HidingDecl*>(d);
      printf("HidingDecl\n");
      break;
    }

  case Decl::SERVER: {
      indent(i, 0);
      VarDecl *x = static_cast<VarDecl*>(d);
      printf("ServerDecl\n");
      break;
    }

  case Decl::RSERVER: {
      indent(i, 0);
      RepServerDecl *x = static_cast<RepServerDecl*>(d);
      printf("RepServerDecl\n");
      break;
    }
  }
}

void Tree::printAbbr(int i, Abbr *a) {
  switch(a->type) {
  
  case Abbr::VAR: {
      indent(i, 0);
      VarAbbr *x = static_cast<VarAbbr*>(a);
      printf("VarAbbr\n");
      break;
    }

  case Abbr::CALL: {
      indent(i, 0);
      CallAbbr *x = static_cast<CallAbbr*>(a);
      printf("CallAbbr\n");
      break;
    }

  case Abbr::SERVER: {
      indent(i, 0);
      ServerAbbr *x = static_cast<ServerAbbr*>(a);
      printf("ServerAbbr\n");
      break;
    }

  case Abbr::PROCESS: {
      indent(i, 0);
      ProcessAbbr *x = static_cast<ProcessAbbr*>(a);
      printf("ProcessAbbr\n");
      break;
    }

  case Abbr::FUNCTION: {
      indent(i, 0);
      FunctionAbbr *x = static_cast<FunctionAbbr*>(a);
      printf("FunctionAbbr\n");
      break;
    }
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

  case Process::CMD: {
      indent(i, 1);
      ProcessCmd *x = static_cast<ProcessCmd*>(p);
      printf("ProcessCmd\n");
      printCmd(i+1, x->cmd);
      break;
    }

  case Process::SPEC: {
      indent(i, 2);
      ProcessSpec *x = static_cast<ProcessSpec*>(p);
      printf("ProcessSpec\n");
      printIntf(i+1, x->intf);
      printCmd(i+1, x->cmd);
      break;
    }

  case Process::INSTANCE: {
      indent(i, 0);
      ProcessInstance *x = static_cast<ProcessInstance*>(p);
      printf("ProcessInstance\n");
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

void Tree::printHidingDecl(int i, HidingDecl *h) {
  indent(i, 0);
  printf("HidingDecl\n");
}

void Tree::printCmd(int i, Cmd *c) {
  switch(c->type) {

  case Cmd::SPEC: {
      indent(i, 2);
      printf("CmdSpec\n");
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
      printf("Ass\n");
      Ass *x = static_cast<Ass*>(c);
      break;
    }

  case Cmd::IN: {
      indent(i, 0);
      printf("In\n");
      In *x = static_cast<In*>(c);
      break;
    }

  case Cmd::OUT: {
      indent(i, 0);
      printf("Out\n");
      Out *x = static_cast<Out*>(c);
      break;
    }

  case Cmd::CONNECT: {
      indent(i, 0);
      printf("Connect\n");
      Connect *x = static_cast<Connect*>(c);
      break;
    }

  case Cmd::ALT: {
      indent(i, 0);
      printf("Alt\n");
      Alt *x = static_cast<Alt*>(c);
      break;
    }

  case Cmd::TEST: {
      indent(i, 0);
      printf("Test\n");
      Test *x = static_cast<Test*>(c);
      break;
    }

  case Cmd::IFD: {
      indent(i, 0);
      printf("IfD\n");
      IfD *x = static_cast<IfD*>(c);
      break;
    }

  case Cmd::IFTE: {
      indent(i, 0);
      printf("IfTE\n");
      IfTE *x = static_cast<IfTE*>(c);
      break;
    }

  case Cmd::CASE: {
      indent(i, 0);
      printf("Case\n");
      Case *x = static_cast<Case*>(c);
      break;
    }

  case Cmd::WHILE: {
      indent(i, 0);
      printf("While\n");
      While *x = static_cast<While*>(c);
      break;
    }

  case Cmd::UNTIL: {
      indent(i, 0);
      printf("Until\n");
      Until *x = static_cast<Until*>(c);
      break;
    }

  case Cmd::DO: {
      indent(i, 0);
      printf("Do\n");
      Do *x = static_cast<Do*>(c);
      break;
    }

  case Cmd::PAR: {
      indent(i, 0);
      printf("Par\n");
      Par *x = static_cast<Par*>(c);
      break;
    }

  case Cmd::INSTANCE: {
      indent(i, 0);
      printf("Instance\n");
      Instance *x = static_cast<Instance*>(c);
      break;
    }

  case Cmd::CALL: {
      indent(i, 0);
      printf("Call\n");
      Call *x = static_cast<Call*>(c);
      break;
    }

  case Cmd::RALT: {
      indent(i, 0);
      printf("RepAlt\n");
      RepAlt *x = static_cast<RepAlt*>(c);
      break;
    }

  case Cmd::RTEST: {
      indent(i, 0);
      printf("RepTest\n");
      RepTest *x = static_cast<RepTest*>(c);
      break;
    }

  case Cmd::RCASE: {
      indent(i, 0);
      printf("RepCase\n");
      RepCase *x = static_cast<RepCase*>(c);
      break;
    }

  case Cmd::RSEQ: {
      indent(i, 0);
      printf("RepSeq\n");
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

