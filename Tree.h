#ifndef TREE_H
#define TREE_H

#include <list>
#include <string>

struct Node;

// The syntax tree base struct
struct Tree {
  std::list<Node*> spec;
  std::list<Node*> prog;
  //void print();
};

// The tree node base struct
struct Node {
};

// Forward declarations
struct Spec;
struct Decl;
struct HidingDecl;
struct Proc;
struct Cmd;
struct Alt;
struct Altn;
struct Choice;
struct Select;
struct Range;
struct Server;
struct Proc;
struct Fml;
struct Expr;
struct Elem;
struct Name;
  
// Specifiers =================================================================

struct Spef : public Node {
  typedef enum {
    VAL,
    VAR,
    CHAN,
    CALL,
    INTF,
    SERVER,
    PROC,
    FUNC
  } Type;
  Type type;
  bool val;
  std::list<Expr*> *lengths;
  Spef(Type t) : 
    type(t), val(false), lengths(NULL) {}
  Spef(Type t, std::list<Expr*> *l) : 
    type(t), val(false), lengths(l) {}
  Spef(Type t, bool v) : 
    type(t), val(v), lengths(NULL) {}
  Spef(Type t, bool v, std::list<Expr*> *l) : 
    type(t), val(v), lengths(l) {}
};

// Interface specifier
struct IntfSpef : public Spef {
  std::list<Decl*> *intf;
  IntfSpef(Type t, std::list<Decl*> *i) :
    Spef(t, false), intf(i) {}
  IntfSpef(Type t, std::list<Decl*> *i, std::list<Expr*> *l) :
    Spef(t, false, l), intf(i) {}
};

// Named specifier
struct NamedSpef : public Spef {
  Name *name;
  NamedSpef(Type t, Name *n) :
    Spef(t, false), name(n) {}
  NamedSpef(Type t, Name *n, std::list<Expr*> *l) :
    Spef(t, false, l), name(n) {}
};

// Formals ====================================================================

struct Fml : public Node {
  typedef enum {
    SINGLE,
    LIST
  } Type;
  Spef *spef;
  Name *name;
  Fml(Spef *s, Name *n) :
    spef(s), name(n) {}
};

// Elements ===================================================================

struct Elem : public Node {
  typedef enum {
    SUBSCRIPT,
    FIELD,
    NAME,
    LITERAL
  } Type;
  Type type;
protected:
  Elem(Type t) :
    type(t) {}
};

// Name
struct Name : public Elem {
  std::string str;
  Name(std::string &s) :
    Elem(NAME), str(s) {}
};

// Specifications =============================================================

struct Spec : public Node {
  typedef enum {
    DEF,
    DECL,
    ABBR,
    SSPEC
  } Type;
  Type type;
  union {
    Name *name;
    std::list<Name*> *names;
  };
protected:
  Spec(Type t, Name *n) : 
    type(t), name(n) {}
  Spec(Type t, std::list<Name*> *n) : 
    type(t), names(n) {}
};

// Definition
struct Def : public Spec {
  typedef enum {
    PROC,
    SERVER,
    ISERVER,
    FUNC
  } DefType;
  DefType defType;
  std::list<Fml*> *args;
protected:
  Def(DefType t, Name *n, std::list<Fml*> *a) :
    Spec(DEF, n), defType(t), args(a) {}
};

// Process definition
struct ProcDef : public Def {
  Proc *proc;
  ProcDef(Name *n, std::list<Fml*> *a, Proc *p) :
    Def(PROC, n, a), process(p) {}
};

// Server definition
struct ServerDef : public Def {
  Server *server;
  Server(Name *n, std::list<Fml*> *a, Server s) :
    Def(SERVER, n, a), server(s) {}
};

// Inheriting server definition
struct InhrtServerDef : public Def {
  std::list<Decl*> *intf;
  HidingDecl *decl; 
  InhrtServDef(Name *n, std::list<Fml*> *a, HidingDecl *d) :
    Def(ISERVER, n, a), decl(d) {}
};

// Function definition
struct FuncDef : public Def {
  Expr *expr;
  FuncDef(Name *n, std::list<Fml*> *a, Expr *e) :
    Def(FUNC, n, a), expr(e) {}
};

// Simultaneous specification
struct SimSpec : public Spec {
  std::list<Spec*> *specs;
  SimSpec(std::list<Spec*> *s) :
    Spec(SSPEC, (Name *) NULL), specs(s) {}
};

// Declaration
struct Decl : public Spec {
  typedef enum {
    VAR,
    ARRAY,
    HIDING,
    SERVER,
    RSERVER
  } DeclType;
  DeclType tDecl;
protected:
  Decl(DeclType t, Name *n) :
    Spec(DECL, n), tDecl(VAR) {}
  Decl(DeclType t, std::list<Name*> *n) :
    Spec(DECL, n), tDecl(VAR) {}
};

// Variable declaration
struct VarDecl : public Decl {
  Spef *spef;
  VarDecl(Spef *s, Name *n) :
    Decl(VAR, n), spef(s) {}
  VarDecl(Spef *s, std::list<Name*> *n) :
    Decl(VAR, n), spef(s) {}
};

// Call declaration
struct CallDecl : public Decl {
  Spef *spef;
  union {
    std::list<Fml*> *args;
    std::list<std::list<Fml*>*> *argss;
  };
  CallDecl(Spef *s, Name *n, std::list<Fml*> *a) :
    Decl(VAR, n), spef(s), args(a) {}
  CallDecl(Spef *s, std::list<Name*> *n, std::list<std::list<Fml*>*> *a) :
    Decl(VAR, n), spef(s), argss(a) {}
};

// Hiding declaration
struct HidingDecl : public Decl {
  std::list<Spec*> *decls;
  Hiding(Name *n, std::list<Spec*> *d) :
    Decl(HIDING, n), decls(d) {}
};

// Server declaration
struct ServerDecl : public Decl {
  Server *server;
  ServerDecl(Name *n, Server *s) :
    Decl(SERVER, n), serv(s) {} 
}

// Replicated server declaration
struct RServerDecl : public Decl {
  Server *server;
  std::list<IndexRange*> exprs
  ServerDecl(Name *n, std::list<IndexRange*> e, Server *s) :
    Decl(RSERVER, n), exprs(e), server(s) {}
}

// Abbreviation
struct Abbr : public Spec {
  typedef enum {
    VAR,
    CALL,
    SERV,
    PROC,
    FUNC
  } Type;
  Type type;
  Spef *spef;
  Elem *elem;
protected:
  Abbr(Type t, Spef *s, Name *n, Elem *e) :
    Spec(Spec::ABBR, n), type(t), spef(s), elem(e) {}
};

// Variable abbreviation
struct VarAbbr : public Abbr {
  VarAbbr(Spef *s, Name *n, Elem* e) :
    Abbr(VAR, s, n, e) {}
};

// Call abbreviation
struct CallAbbr : public Abbr {
  std::list<Fml*> *args;
  CallAbbr(Spef *s, Name *n, std::list<Fml*> *a, Elem* e) :
    Abbr(CALL, s, n, e), args(a) {}
};

// Server abbreviation
struct ServerAbbr() : public Abbr {
  ServerAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(SERV, s, n, e) {}
};

// Process abbreviation
struct ProcAbbr : public Abbr {
  ProcAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(PROC, s, n, e) {}
};

// Function abbreviation
struct FuncAbbr() : public Abbr {
  FuncAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(FUNC, s, n, e) {}
};

// Commands ===================================================================

struct Cmd : public Node {
  typedef enum {
    SPEC,
    // Procedures and calls
    INSTANCE,
    CALL,
    // Primitive
    SKIP,
    STOP,
    ASS,
    IN,
    OUT,
    CONN,
    // Structured
    ALT,
    TEST,
    IFD,
    IFTE,
    CASE,
    WHILE,
    UNTIL,
    DO,
    SEQ,
    PAR,
    // Replicated
    RALT,
    RTEST,
    RCASE,
    RSEQ
  } Type;
  Type type;
protected:
  Cmd(Type t) :
    type(t) {}
};

// Command specification
struct CmdSpec : public Cmd {
  Spec *spec;
  Cmd *cmd;
  CmdSpec(Spec *s, Cmd *c) :
    Cmd(SPEC), spec(s), cmd(c) {}
};

// Instance
struct Instance : public Cmd {
  Name *name;
  std::list<Expr*> *actuals;
  Instance(Name *n, std::list<Expr*> *a) :
    Cmd(INSTANCE), name(n), actuals(a) {}
};

// Call
struct Call : public Cmd {
  Name *name;
  Name *field;
  std::list<Expr*> *actuals;
  Call(Name *n, Name *f, std::list<Expr*> *a) : 
    Cmd(CALL), name(n), field(f), actuals(a) {}
};

// Replicated sequence
struct RSeq : public Cmd {
  std::list<Range*> *ranges;
  RSeq(std::list<Range*> *r) :
    Cmd(RSEQ), ranges(r) {}
};

// Skip
struct Skip : public Cmd {
  Skip() : 
    Cmd(SKIP) {}
};

// Stop
struct Stop : public Cmd {
  Stop() : 
    Cmd(STOP) {}
};

// Assignment
struct Ass : public Cmd {
  Elem *lhs;
  Expr *rhs;
  Ass(Elem *lhs, Expr *rhs) : 
    Cmd(ASS), lhs(lhs),  rhs(rhs) {}
};

// Input
struct In : public Cmd {
  Elem *lhs, *rhs;
  In(Elem *lhs, Elem *rhs) : 
    Cmd(IN), lhs(lhs), rhs(rhs) {}
};

// Output
struct Out : public Cmd {
  Elem *lhs;
  Expr *rhs;
  Out(Elem *lhs, Expr *rhs) : 
    Cmd(OUT), lhs(lhs), rhs(rhs) {}
};

// Connect
struct Connect : public Cmd {
  Elem *local;
  Elem *remote;
  Connect(Elem *l, Elem *r) : 
    Cmd(CONN), local(l), remote(r) {}
};

// Alternative
struct Alt : public Cmd {
  std::list<Altn*> *altns;
  Alt(std::list<Altn*> *a) : 
    Cmd(ALT), altns(a) {}
};

struct RepAlt : public Cmd {
  std::list<Range*> *ranges;
  Altn *altn;
  RepAlt(std::list<Range*> *r, Altn *a) : 
    Cmd(RALT), ranges(r), altn(a) {}
};

// Alternation
struct Altn {
  typedef enum {
    UNGUARDED,
    GUARDED,
    SKIP,
    NESTED,
    SPEC
  } Type;
  Type type;
protected:
  Altn(Type t) : type(t) {};
};

struct UnguardedAltn : public Altn {
  Elem *dst;
  Elem *src;
  Cmd *cmd;
  UnguardedAltn(Elem *d, Elem *s, Cmd *c) :
    Altn(UNGUARDED), dst(d), src(s), cmd(c) {}
};

struct GuardedAltn : public Altn {
  Expr *expr;
  Elem *dst;
  Elem *src;
  Cmd *cmd;
  GuardedAltn(Expr *e, Elem *d, Elem *s, Cmd *c) :
    Altn(GUARDED), expr(e), dst(d), src(s), cmd(c) {}
};

struct SkipAltn : public Altn {
  Expr *expr;
  Cmd *cmd;
  SkipAltn(Expr *e, Cmd *c) :
    Altn(SKIP), expr(e), cmd(c) {}
};

struct NestedAltn : public Altn {
  Alt *alt;
  NestedAltn(Alt *a) :
    Altn(NESTED), alt(a) {}
};

struct SpecAltn : public Altn {
  Spec *spec;
  Altn *altn;
  SpecAltn(Spec *s, Altn *a) :
    Altn(SPEC), spec(s), altn(a) {}
};

// Conditional
struct Test : public Cmd {
  std::list<Choice*> *choices;
  Test(std::list<Choice*> *c) : 
    Cmd(TEST), choices(c) {}
};

struct RepTest : public Cmd {
  std::list<Range*> *ranges;
  Choice *choice;
  RepTest(std::list<Range*> *r, Choice *c) : 
    Cmd(RTEST), ranges(r), choice(c) {}
};

struct IfD : public Cmd {
  Expr *expr;
  Cmd *cmd;
  IfD(Expr *e, Cmd *c) :
    Cmd(IFD), expr(e), cmd(c) {}
};

struct IfTE : public Cmd {
  Expr *expr;
  Cmd *cmd;
  Cmd *elseCmd;
  IfTE(Expr *e, Cmd *c, Cmd *d) :
    Cmd(IFTE), expr(e), cmd(c), elseCmd(d) {}
};

// Choice
struct Choice {
  typedef enum {
    GUARDED,
    NESTED,
    SPEC
  } Type;
  Type type;
protected:
  Choice(Type t) : type(t)  {};
};

struct GuardedChoice : public Choice {
  Expr *expr;
  Cmd *cmd;
  GuardedChoice(Expr *e, Cmd *c) : 
    Choice(GUARDED), expr(e), cmd(c)  {};
};

struct NestedChoice : public Choice {
  Test *test;
  NestedChoice(Test *t) : 
    Choice(NESTED), test(t)  {};
};

struct SpecChoice : public Choice {
  Spec *spec;
  Choice *choice;
  SpecChoice(Spec *s, Choice *c) : 
    Choice(SPEC), spec(s), choice(c) {}
};

// Case
struct Case : public Cmd {
  Expr *expr;
  std::list<Select*> *selects;
  Case(Expr *e, std::list<Select*> *s) : 
    Cmd(CASE), expr(e), selects(s) {}
};

struct RepCase : public Cmd {
  Expr *expr;
  std::list<Range*> *ranges;
  Select *select;
  RepCase(Expr *e, std::list<Range*> *r, Select *s) : 
    Cmd(RCASE), expr(e), ranges(r), select(s) {}
};

// Select
struct Select {
  typedef enum {
    GUARDED,
    ELSE
  } Type;
  Type type;
  Cmd *cmd;
protected:
  Select(Type t, Cmd *c) : type(t), cmd(c) {}
};

struct GuardedSelect : public Select {
  Expr *expr;
  GuardedSelect(Expr *e, Cmd *c) :
    Select(GUARDED, c), expr(e) {}
};

struct ElseSelect : public Select {
  ElseSelect(Cmd *c) :
    Select(ELSE, c) {}
};

// Loop
struct While : public Cmd {
  Expr *expr;
  Cmd *cmd;
  While(Expr *e, Cmd *c) :
    Cmd(WHILE), expr(e), cmd(c) {}
};

struct Do : public Cmd {
  Cmd *cmd;
  Expr *expr;
  Do(Cmd *c, Expr *e) :
    Cmd(DO), cmd(c), expr(e) {}
};

struct Until : public Cmd {
  Expr *expr;
  Cmd *cmd;
  Until(Expr *e, Cmd *c) :
    Cmd(UNTIL), expr(e), cmd(c) {}
};

// Sequence
struct Seq : public Cmd {
  std::list<Cmd*> *cmds;
  Seq(std::list<Cmd*> *c) : 
    Cmd(SEQ), cmds(c) {}
};

// Parallel
struct Par : public Cmd {
  Par() : 
    Cmd(PAR) {}
};

// Index range
struct Range {
  Name *name;
  Expr *base;
  Expr *count;
  Expr *step;
  Range(Name *n, Expr *b, Expr *c, Expr *s) :
    name(n), base(b), count(c), step(s) {}
};

// Server entity
struct Server {
  typedef enum {
    SPEC,
    INSTANCE
  } Type;
  Server(Type t) {}
};

struct ServerSpec : public Server {
  std::list<Decl*> *intfs;
  std::list<Decl*> *decls;
  Server(std::list<Decl*> *i, std::list<Decl*> *d) :
    Server(SPEC), intfs(i), decls(d) {}
};

struct ServerInstance : public Server {
  Name *name;
  std::list<Expr*> *actuals;
  ServerInstance(Name *n, std::list<Expr*> *a) :
    Server(INSTANCE), name(n), actuals(a) {}
};

// Process entity
struct Proc {
  typedef enum {
    CMD,
    SPEC,
    INSTANCE
  } Type;
  Proc(Type t) {}
};

struct ProcCmd : public Proc {
  Cmd *cmd;
  ProcCmd(Cmd *c) :
    Proc(CMD), cmd(c) {}
};

struct ProcSpec : public Proc {
  std::list<Decl*> *intf;
  Cmd *cmd;
  ProcSpec(std::list<Decl*> *i, Cmd *c) :
    Proc(SPEC), intf(i), cmd(c) {}
};

struct ProcInstance : public Proc {
  Name *name;
  std::list<Expr*> *actuals;
  ProcInstance(Name *n, std::list<Expr*> *a) :
    Proc(INSTANCE), name(n), actuals(a) {}
};

// Expressions ================================================================

struct Expr : public Node {
  typedef enum {
    UNARY,
    BINARY,
    VALOF
  } Type;
protected:
  Expr() {}
};

#endif
