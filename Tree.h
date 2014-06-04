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
struct Cmd;
struct Alt;
struct Altn;
struct Choice;
struct Range;
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
    PROC,
    SERV,
    FUNC
  } Type;
  Type type;
  bool val;
  std::list<Expr*> *lengths;
  Spef(Type t, bool v, std::list<Expr*> *l) : 
    type(t), val(v), lengths(NULL) {}
};

// Interface specifier
struct IntfSpef : public Spef {
  std::list<Decl*> *intf;
  IntfSpef(Type t, std::list<Decl*> *i, std::list<Expr*> *l) :
    Spef(t, false, l), intf(i) {}
};

// Named specifier
struct NamedSpef : public Spef {
  Name *name;
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
  union {
    Name *name;
    std::list<Name*> *names;
  };
  Fml(Spef *s, Name *n) :
    spef(s), name(n) {}
  Fml(Spef *s, std::list<Name*> *n) :
    spef(s), names(n) {}
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
    SDEF,
    DECL,
    ABBR
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
    SERV,
    FUNC
  } DefType;
  DefType defType;
  std::list<Fml*> *args;
protected:
  Def(DefType t, Name *n, std::list<Fml*> *a) :
    Spec(DEF, n), defType(t), args(a) {}
};

// Process definition
struct Proc : public Def {
  std::list<Decl*> *intf;
  Cmd *cmd;
  Proc(Name *n, std::list<Fml*> *a, std::list<Decl*> *i, Cmd *c) :
    Def(PROC, n, a), intf(i), cmd(c) {}
};

// Server definition
struct Serv : public Def {
  Cmd *body;
  std::list<Decl*> *intf;
  std::list<Decl*> *decls;
  Serv(Name *n, std::list<Fml*> *a, std::list<Decl*> *i, std::list<Decl*> *d) :
    Def(SERV, n, a), intf(i), decls(d) {}
};

// Function definition
struct Func : public Def {
  Expr *expr;
  Func(Name *n, std::list<Fml*> *a, Expr *e) :
    Def(FUNC, n, a), expr(e) {}
};

// Simultaneous definition
struct SimDef : public Spec {
  std::list<Def*> *defs;
  SimDef(std::list<Def*> *d) :
    Spec(SDEF, (Name *) NULL), defs(d) {}
};

// Declaration
struct Decl : public Spec {
  typedef enum {
    VAR,
    ARRAY
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

// Abbreviation
struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR
  } Type;
  Type type;
protected:
  Abbr(Type t, Name *n) :
    Spec(Spec::ABBR, n), type(t) {}
};

// Value abbreviation 
struct ValAbbr : public Abbr {
  Expr *expr;
  ValAbbr(Name *n, Expr *e) :
    Abbr(Abbr::VAL, n), expr(e) {}
};

// Variable abbreviation
struct VarAbbr : public Abbr {
  Spef *spef;
  Elem *elem;
  VarAbbr(Spef *s, Name *n, Elem* e) :
    Abbr(Abbr::VAR, n), spef(s), elem(e) {}
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
    RALT,
    TEST,
    RTEST,
    IFD,
    IFTE,
    WHILE,
    UNTIL,
    DO,
    SEQ,
    RSEQ,
    PAR
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
  Elem *src;
  Elem *dst;
  Cmd *cmd;
  UnguardedAltn(Elem *s, Elem *d, Cmd *c) :
    Altn(UNGUARDED), src(s), dst(d), cmd(c) {}
};

struct GuardedAltn : public Altn {
  Expr *expr;
  Elem *src;
  Elem *dst;
  Cmd *cmd;
  GuardedAltn(Expr *e, Elem *s, Elem *d, Cmd *c) :
    Altn(GUARDED), expr(e), src(s), dst(d), cmd(c) {}
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

// Loop
struct While : public Cmd {
  Expr *cond;
  Cmd *body;
  While(Expr *e, Cmd *c) :
    Cmd(WHILE), cond(e), body(c) {}
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
