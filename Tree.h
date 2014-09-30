#ifndef TREE_H
#define TREE_H

#include "Lex.h"

#include <list>
#include <string>

// Forward declarations
struct Node;
struct Def;
struct Spec;
struct Decl;
struct Abbr;
struct HidingDecl;
struct Cmd;
struct Alt;
struct Altn;
struct Choice;
struct Select;
struct Range;
struct Server;
struct Process;
struct Fmls;
struct Fml;
struct Elem;
struct Name;
struct Expr;
struct Literal;
struct Valof;

// The syntax tree
struct Tree {
public:
  std::list<Spec*> spec;
  std::list<Cmd*> prog;
  void print();

private:
  void printSpec(int x, Spec*);
  void printDef(int x, Def*);
  void printDecl(int x, Decl*);
  void printAbbr(int x, Abbr*);
  void printFmls(int x, std::list<Fml*>*);
  void printFml(int x, Fml*);
  void printProcess(int x, Process*);
  void printServer(int x, Server*);
  void printIntf(int x, std::list<Decl*>*);
  void printHidingDecl(int x, HidingDecl*);
  void printCmd(int x, Cmd*);
  void printExpr(int x, Expr*);
  void printName(int x, Name*);
};

// The tree node base struct
struct Node {};

// ============================================================================
// Specifiers 
// ============================================================================

struct Spef : public Node {
  typedef enum {
    VAL,
    VAR,
    CHAN,
    CALL,
    INTF,
    SERVER,
    PROCESS,
    FUNCTION
  } Type;
  Type type;
  bool val;
  std::list<Expr*> *lengths;
  Spef(Type t) : 
    type(t), val(false), lengths(nullptr) {}
  Spef(Type t, std::list<Expr*> *l) : 
    type(t), val(false), lengths(l) {}
  Spef(Type t, bool v) : 
    type(t), val(v), lengths(nullptr) {}
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

// ============================================================================
// Formals
// ============================================================================

struct Fml : public Node {
  Spef *spef;
  Name *name;
  Fml(Spef *s, Name *n) :
    spef(s), name(n) {}
};

// ============================================================================
// Elements
// ============================================================================

struct Elem : public Node {
  typedef enum {
    NAME,
    FIELD,
    LITERAL,
    SUBSCRIPT
  } Type;
  Type type;
  std::list<Expr*> *subscripts;

protected:
  Elem(Type t) :
    type(t), subscripts(nullptr) {}
  Elem(Type t, std::list<Expr*> *s) :
    type(t), subscripts(s) {}
};

// Name
struct Name : public Elem {
  std::string str;
  Name(std::string n) :
    Elem(NAME), str(n) {}
  Name(std::string n, std::list<Expr*> *s) :
    Elem(NAME, s), str(n) {}
};

// Field
struct Field : public Elem {
  Name *base;
  Name *field;
  Field(Name *b, Name *f) :
    Elem(FIELD), base(b), field(f) {}
  Field(Name *b, Name *f, std::list<Expr*> *s) :
    Elem(FIELD, s), base(b), field(f) {}
};

// ============================================================================
// Specifications
// ============================================================================

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
  bool nameList;

protected:
  Spec(Type t, Name *n) : 
    type(t), name(n), nameList(false) {}
  Spec(Type t, std::list<Name*> *n) : 
    type(t), names(n), nameList(true) {}
};

// Definition
struct Def : public Spec {
  typedef enum {
    PROCESS,
    SERVER,
    ISERVER,
    FUNCTION
  } DefType;
  DefType defType;
  std::list<Fml*> *args;

protected:
  Def(DefType t, Name *n, std::list<Fml*> *a) :
    Spec(DEF, n), defType(t), args(a) {}
};

// Process definition
struct ProcessDef : public Def {
  Process *process;
  ProcessDef(Name *n, std::list<Fml*> *a, Process *p) :
    Def(PROCESS, n, a), process(p) {}
};

// Server definition
struct ServerDef : public Def {
  Server *server;
  ServerDef(Name *n, std::list<Fml*> *a, Server *s) :
    Def(SERVER, n, a), server(s) {}
};

// Inheriting server definition
struct InhrtServerDef : public Def {
  std::list<Decl*> *intf;
  HidingDecl *hidingDecl; 
  InhrtServerDef(Name *n, std::list<Fml*> *a, HidingDecl *h) :
    Def(ISERVER, n, a), hidingDecl(h) {}
};

// Function definition
struct FunctionDef : public Def {
  Expr *expr;
  FunctionDef(Name *n, std::list<Fml*> *a, Expr *e) :
    Def(FUNCTION, n, a), expr(e) {}
};

// Simultaneous specification
struct SimSpec : public Spec {
  std::list<Spec*> *specs;
  SimSpec(std::list<Spec*> *s) :
    Spec(SSPEC, (Name *) nullptr), specs(s) {}
};

// Declaration
struct Decl : public Spec {
  typedef enum {
    VAR,
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
  HidingDecl(Name *n, std::list<Spec*> *d) :
    Decl(HIDING, n), decls(d) {}
};

// Server declaration
struct ServerDecl : public Decl {
  Server *server;
  ServerDecl(Name *n, Server *s) :
    Decl(SERVER, n), server(s) {} 
};

// Replicated server declaration
struct RepServerDecl : public Decl {
  Server *server;
  std::list<Range*> *exprs;
  RepServerDecl(Name *n, std::list<Range*> *e, Server *s) :
    Decl(RSERVER, n), server(s), exprs(e) {}
};

// Abbreviation
struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR,
    CALL,
    SERVER,
    PROCESS,
    FUNCTION
  } Type;
  Type type;
  Spef *spef;
  union {
    Elem *elem;
    Expr *expr;
  }

protected:
  Abbr(Type t, Spef *s, Name *n, Expr *e) :
    Spec(Spec::ABBR, n), type(t), spef(s), expr(e) {}
  Abbr(Type t, Spef *s, Name *n, Elem *e) :
    Spec(Spec::ABBR, n), type(t), spef(s), elem(e) {}
};

// Value abbreviation
struct ValAbbr : public Abbr {
  ValAbbr(Spef *s, Name *n, Expr* e) :
    Abbr(VAL, s, n, e) {}
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
struct ServerAbbr : public Abbr {
  ServerAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(SERVER, s, n, e) {}
};

// Process abbreviation
struct ProcessAbbr : public Abbr {
  ProcessAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(PROCESS, s, n, e) {}
};

// Function abbreviation
struct FunctionAbbr : public Abbr {
  FunctionAbbr(Spef *s, Name *n, Elem *e) :
    Abbr(FUNCTION, s, n, e) {}
};

// ============================================================================
// Commands
// ============================================================================

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
    CONNECT,
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
    Cmd(CONNECT), local(l), remote(r) {}
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

// Replicated sequence
struct RepSeq : public Cmd {
  std::list<Range*> *ranges;
  RepSeq(std::list<Range*> *r) :
    Cmd(RSEQ), ranges(r) {}
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
  Type type;
  Server(Type t) :
    type(t) {}
};

struct ServerSpec : public Server {
  std::list<Decl*> *intfs;
  std::list<Spec*> *decls;
  ServerSpec(std::list<Decl*> *i, std::list<Spec*> *d) :
    Server(SPEC), intfs(i), decls(d) {}
};

struct ServerInstance : public Server {
  Name *name;
  std::list<Expr*> *actuals;
  ServerInstance(Name *n, std::list<Expr*> *a) :
    Server(INSTANCE), name(n), actuals(a) {}
};

// Process entity
struct Process {
  typedef enum {
    CMD,
    SPEC,
    INSTANCE
  } Type;
  Type type;
  Process(Type t) : 
    type(t) {}
};

struct ProcessCmd : public Process {
  Cmd *cmd;
  ProcessCmd(Cmd *c) :
    Process(CMD), cmd(c) {}
};

struct ProcessSpec : public Process {
  std::list<Decl*> *intf;
  Cmd *cmd;
  ProcessSpec(std::list<Decl*> *i, Cmd *c) :
    Process(SPEC), intf(i), cmd(c) {}
};

struct ProcessInstance : public Process {
  Name *name;
  std::list<Expr*> *actuals;
  ProcessInstance(Name *n, std::list<Expr*> *a) :
    Process(INSTANCE), name(n), actuals(a) {}
};

// ============================================================================
// Expressions
// ============================================================================

struct Expr : public Node {
  typedef enum {
    UNARY,
    BINARY,
    ELEM,
    LITERAL,
    VALOF,
    EXPR
  } Type;
  Type type;

protected:
  Expr(Type t) : type(t) {}
};

struct Operand : public Expr {
  Operand(Type t) : Expr(t) {}
};

struct OperElem : public Operand {
  Elem *elem;
  OperElem(Elem *e) : 
    Operand(ELEM), elem(e) {};
};

struct OperLiteral : public Operand {
  Literal *literal;
  OperLiteral(Literal *l) : 
    Operand(LITERAL), literal(l) {}
};

struct OperValof : public Operand {
  Valof *valof;
  OperValof(Valof *v) : 
    Operand(VALOF), valof(v) {}
};

struct OperExpr : public Operand {
  Expr *expr;
  OperExpr(Expr *e) : 
    Operand(EXPR), expr(e) {}
};

struct UnaryOp : public Expr {
  Lex::Token op;
  Operand *operand;
  UnaryOp(Lex::Token t, Operand *o) :
    Expr(UNARY), op(t), operand(o) {}
};

struct BinaryOp : public Expr {
  Lex::Token op;
  Operand *left;
  Operand *right;
  BinaryOp(Lex::Token t, Operand *l, Operand *r) :
    Expr(BINARY), op(t), left(l), right(r) {}
};
    
struct Literal {
  typedef enum {
    DECINT,
    HEXINT,
    OCTINT,
    BININT,
    CHAR,
    BOOL
  } Type;
  Type type;
  Literal(Type t) : type(t) {}
};

struct DecIntLiteral : public Literal {
  int value;
  DecIntLiteral(int v) :
    Literal(DECINT), value(v) {}
};

struct HexIntLiteral : public Literal {
  int value;
  HexIntLiteral(int v) :
    Literal(HEXINT), value(v) {}
};

struct OctIntLiteral : public Literal {
  int value;
  OctIntLiteral(int v) :
    Literal(OCTINT), value(v) {}
};

struct BinIntLiteral : public Literal {
  int value;
  BinIntLiteral(int v) :
    Literal(BININT), value(v) {}
};

struct CharLiteral : public Literal {
  char value;
  CharLiteral(char v) :
    Literal(CHAR), value(v) {}
};

struct BoolLiteral : public Literal {
  bool value;
  BoolLiteral(bool v) :
    Literal(BOOL), value(v) {}
};

struct Valof : public Expr {
  Cmd *cmd;
  Expr *expr;
  Valof(Cmd *c, Expr *e) :
    Expr(VALOF), cmd(c), expr(e) {}
};

#endif

