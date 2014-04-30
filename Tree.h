#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

struct Node;

// The syntax tree base struct
struct Tree {
  std::vector<Node*> spec;
  std::vector<Node*> prog;
  void print();
};

// The tree node base struct
struct Node {
};

// Base Tree Node types
struct Spec;
struct Cmd;
struct Formal;
struct Expr;
struct Elem;
struct Name;
  
// Specifiers =================================================================

struct Spef : public Node {
  typedef enum {
    VAR,
    VAL
  } Type;
  Type type;
  Spec(Type t) : type(t) {}
};

struct ArraySpef : public Spef {
  std::vector<Expr*> *lengths;
  ArrayType(Type t, 
            std::vector<Expr*> *l) :
    type(t),
    lengths(l) {}
};

// Formals ====================================================================

struct Formal : public Node {
  Spef *spef;
  Name *name;
  Formal(Spef *s, 
      Name *n) :
    spef(s),
    name(n) {}
};

struct FormalMult : public Node {
  Spef *spef;
  std::vector<Name*> names;
  FormalMult(Spef *s,
             std::vector<Name*> n*) :
    spef(s),
    name(n)
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
  Elem(Type t) :
    type(t) {}
};

struct Name : public Elem {
  std::string str;
  Name(std::string &s) :
    Elem(Elem::NAME),
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
  } Type;
  std::vector<Formal*> *args;
  Cmd *body;
  Def(Type t,
      Name *n,
      std::vector<Formal*> *f,
      Cmd *c) :
    Spec(Spec::DEF, name),
    type(t),
    args(f), 
    body(c) {}
};

struct Decl : public Spec {
  typedef enum {
    VAR,
    ARRAY
  } Type;
  Spef *spef;
  Decl(Spef *s,
       Name *n) :
    Spec(Spec::DECL, n, s),
    type(VAR),
    name(n),
    spef(s) {}
};

struct ArrayDecl : public Spec {
  ArraySpef *spef;
  ArrayDecl(Name *n,
            ArraySpef *s) :
    Spec(Spec::DECL, n),
    type(Decl::ARRAY),
    spef(s) {}
};

struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR
  } Type;
  Type type;
  Abbr(Type t, 
       Name *n) :
    Spec(Spec::ABBR, n), 
    type(t) {}
};

struct ValAbbr : public Abbr {
  Expr *expr;
  ValAbbr(Name *name,
          Elem *elem) :
    Abbr(Abbr::VAL, n),
    elem(elem) {}
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
    elem(elem) {}
};

// Commands ===================================================================

struct Cmd : public Node {
  typedef enum {
    SKIP,
    STOP,
    ASS,
    IN,
    OUT,
    CONNECT,
    ALT,
    COND,
    IF_THEN_ELSE,
    LOOP,
    SEQ,
    PAR
  } Type;
  Type type;
  Cmd(Type type) : type(type) {}
};

struct Skip : public Cmd {
  Skip() : Cmd(Cmd::SKIP) {}
};

struct Stop : public Cmd {
  Stop() : Cmd(Cmd::STOP) {}
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
  Elem *local, *remote;
  Connect(Elem *local, Elem *remote) : 
    Cmd(Cmd::CONNECT),
    local(local),
    remote(remote) {}
};

struct Alternative {
};

struct Alt : public Cmd {
  std::vector<Alternative> *alternatives;
  Alt(std::vector<Alternative> *alternatives) : 
    Cmd(Cmd::ALT),
    alternatives(alternatives) {}
};

struct Choice {
};

struct Cond : public Cmd {
  std::vector<Choice> *choices;
  Cond(std::vector<Choice> *choices) : 
    Cmd(Cmd::COND),
    choices(choices) {}
};

struct IfThenElse : public Cmd {
  Cmd *ifCmd, *elseCmd;
  IfThenElse() : Cmd(Cmd::IF_THEN_ELSE) {
  }
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
  Par() : Cmd(Cmd::PAR) {
  }
};


// Expressions ================================================================

struct Expr : public Node {
  typedef enum {
    UNARY,
    BINARY,
    VALOF
  } Type;
};

#endif
