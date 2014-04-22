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

// Elements ===================================================================

struct Elem : public Node {
  typedef enum {
    SUBSCRIPT,
    FIELD,
    NAME,
    LITERAL
  } Type;
  Type type;
  Elem(Type type) :
    type(type) {}
};

struct Name : public Elem {
  std::string s;
  Name(const std::string &s) :
    Elem(Elem::NAME),
    s(s) {}
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
  Spec(Type type, Name *name) : 
    type(type), name(name) {}
};

struct Def : public Spec {
  typedef enum {
    SERVER,
    PROCESS,
    FUNCTION
  } Type;
  std::vector<Formal*> *args;
  Cmd *body;
  Def(Type type,
      Name *name, 
      std::vector<Formal*> *args, 
      Cmd *body) : 
    Spec(Spec::DEF, name),
    args(args), 
    body(body) {}
};

struct Decl : public Spec {
  typedef enum {
    VAR,
    ARRAY
  } Type;
  Type type;
  Decl(Type type, Name *name) :
    Spec(Spec::DECL, name), type(type) {}
};

struct VarDecl : public Decl {
  VarDecl(Name *name) :
    Decl(Decl::VAR, name) {}
};

struct ArrayDecl : public Decl {
  std::vector<Expr*> sizes;
  ArrayDecl(Name *name, 
            const std::vector<Expr*> &sizes) :
    Decl(Decl::ARRAY, name), sizes(sizes) {}
};

struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR
  } Type;
  Type type;
  Abbr(Type type, Name *name) :
    Spec(Spec::ABBR, name), type(type) {}
};

struct ValAbbr : public Abbr {
  Elem *elem;
  ValAbbr(Name *name, Elem *elem) :
    Abbr(Abbr::VAL, name), elem(elem) {}
};

struct VarAbbr : public Abbr {
  Expr *expr;
  VarAbbr(Name *name, Expr* expr) :
    Abbr(Abbr::VAR, name), expr(expr) {}
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
  std::vector<Alternative> alternatives;
  Alt(std::vector<Alternative> alternatives) : 
    Cmd(Cmd::ALT),
    alternatives(alternatives) {}
};

struct Choice {
};

struct Cond : public Cmd {
  std::vector<Choice> choices;
  Cond(std::vector<Choice> choices) : 
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
  Seq(std::vector<Cmd*> cmds) : 
    Cmd(Cmd::SEQ),
    cmds(cmds) {}
};

struct Par : public Cmd {
  Par() : Cmd(Cmd::PAR) {
  }
};


// Formals ====================================================================

struct Formal : public Node {
  typedef enum {
    VAR,
    VAR_ARRAY,
    CALL_ARRAY,
    VAL
  } Type;
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
