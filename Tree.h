#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

struct Node;

// The syntax tree base struct
struct Tree {
  std::vector<Node*> spec;
  std::vector<Node*> prog;
  //void print();
};

// The tree node base struct
struct Node {
};

// Base Tree Node types
struct Spec;
struct Cmd;
struct Fml;
struct Expr;
struct Elem;
struct Name;
  
// Specifiers =================================================================

struct Spef : public Node {
  typedef enum {
    VAR,
    VAL,
    ARRAY
  } Type;
  Type type;
protected:
  Spef(Type t) : type(t) {}
};

struct VarSpef : public Spef {
  VarSpef() :
    Spef(VAR) {}
};

struct ValSpef : public Spef {
  ValSpef() :
    Spef(VAL) {}
};

struct ArraySpef : public Spef {
  std::vector<Expr*> *lengths;
  ArraySpef(Type t, 
            std::vector<Expr*> *l) :
    Spef(t),
    lengths(l) {}
};

// Formals ====================================================================

struct Fml : public Node {
  typedef enum {
    SINGLE,
    LIST
  } Type;
  Spef *spef;
protected:
  Fml(Spef *s) :
    spef(s) {}
};

struct FmlSingle : public Fml {
  Name *name;
  FmlSingle(Spef *s,
            Name *n) :
    Fml(s),
    name(n) {}
};

struct FmlList : public Fml {
  std::vector<Name*> names;
  FmlList(Spef *s,
          std::vector<Name*> n) :
    Fml(s),
    names(n) {}
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

struct Name : public Elem {
  std::string str;
  Name(std::string &s) :
    Elem(NAME),
    str(s) {}
};

// Specifications =============================================================

struct Spec : public Node {
  typedef enum {
    DEF,
    DECL,
    ABBR
  } Type;
  Type type;
  Name *name;
protected:
  Spec(Type t, 
       Name *n) : 
    type(t), 
    name(n) {}
};

struct Def : public Spec {
  typedef enum {
    SERVER,
    PROCESS,
    FUNCTION
  } DefType;
  DefType tDef;
  std::vector<Fml*> *args;
  Cmd *body;
  Def(DefType t,
      Name *n,
      std::vector<Fml*> *f,
      Cmd *c) :
    Spec(DEF, name),
    tDef(t),
    args(f), 
    body(c) {}
};

struct Decl : public Spec {
  typedef enum {
    VAR,
    ARRAY
  } DeclType;
  DeclType tDecl;
protected:
  Decl(DeclType t, 
       Name *n) :
    Spec(DECL, n),
    tDecl(VAR) {}
};

struct VarDecl : public Decl {
  Spef *spef;
  VarDecl(Spef *s,
          Name *n) :
    Decl(VAR, n),
    spef(s) {}
};

struct ArrayDecl : public Decl {
  ArraySpef *spef;
  ArrayDecl(Name *n,
            ArraySpef *s) :
    Decl(ARRAY, n),
    spef(s) {}
};

struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR
  } Type;
  Type type;
protected:
  Abbr(Type t, 
       Name *n) :
    Spec(Spec::ABBR, n), 
    type(t) {}
};

struct ValAbbr : public Abbr {
  Expr *expr;
  ValAbbr(Name *n,
          Expr *e) :
    Abbr(Abbr::VAL, n),
    expr(e) {}
};

struct VarAbbr : public Abbr {
  Spef *spef;
  Elem *elem;
  VarAbbr(Spef *s,
          Name *n,
          Elem* e) :
    Abbr(Abbr::VAR, n),
    spef(s),
    elem(e) {}
};

struct ArrayAbbr : public Abbr {
  ArraySpef *spef;
  Elem *elem;
  ArrayAbbr(ArraySpef *s,
            Name *n, 
            Elem* e) :
    Abbr(Abbr::VAR, n),
    spef(s),
    elem(e) {}
};

// Commands ===================================================================

struct Cmd : public Node {
  typedef enum {
    SKIP,
    STOP,
    ASS,
    IN,
    OUT,
    CONN,
    ALT,
    COND,
    IFTE,
    LOOP,
    SEQ,
    PAR
  } Type;
  Type type;
protected:
  Cmd(Type t) :
    type(t) {}
};

struct Skip : public Cmd {
  Skip() : 
    Cmd(SKIP) {}
};

struct Stop : public Cmd {
  Stop() : 
    Cmd(STOP) {}
};

struct Ass : public Cmd {
  Elem *lhs;
  Expr *rhs;
  Ass(Elem *lhs, Expr *rhs) : 
    Cmd(Cmd::ASS),
    lhs(lhs), 
    rhs(rhs) {}
};

struct In : public Cmd {
  Elem *lhs, *rhs;
  In(Elem *lhs, Elem *rhs) : 
    Cmd(Cmd::IN),
    lhs(lhs),
    rhs(rhs) {}
};

struct Out : public Cmd {
  Elem *lhs;
  Expr *rhs;
  Out(Elem *lhs, Expr *rhs) : 
    Cmd(Cmd::OUT),
    lhs(lhs),
    rhs(rhs) {}
};

struct Connect : public Cmd {
  Elem *local;
  Elem *remote;
  Connect(Elem *l, Elem *r) : 
    Cmd(CONN),
    local(l),
    remote(r) {}
};

struct Altn {
};

struct Alt : public Cmd {
  std::vector<Altn> *altns;
  Alt(std::vector<Altn> *altns) : 
    Cmd(Cmd::ALT),
    altns(altns) {}
};

struct Choice {
};

struct Cond : public Cmd {
  std::vector<Choice> *choices;
  Cond(std::vector<Choice> *choices) : 
    Cmd(Cmd::COND),
    choices(choices) {}
};

struct IfTE : public Cmd {
  Cmd *ifCmd, *elseCmd;
  IfTE() :
    Cmd(Cmd::IFTE) {}
};

struct Loop : public Cmd {
  Expr *cond;
  Cmd *body;
  Loop(Expr *cond, Cmd *body) : 
    Cmd(Cmd::LOOP),
    cond(cond),
    body(body) {}
};

struct Seq : public Cmd {
  std::vector<Cmd*> cmds;
  Seq(std::vector<Cmd*> &cmds) : 
    Cmd(Cmd::SEQ),
    cmds(cmds) {}
};

struct Par : public Cmd {
  Par() : 
    Cmd(Cmd::PAR) {}
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
