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
struct Node;
struct Spec;
struct Cmd;
struct Formal;
struct Expr;
struct Elem;

// Specifications =============================================================

struct Spec : public Node {
  typedef enum {
    DEF,
    DECL,
    ABBR
  } Type;
  Type type;
  std::string name;
  Spec(Type type, const std::string &name) : 
    type(type), name(name) {}
};

struct Def : public Spec {
  typedef enum {
    SERVER,
    PROCESS,
    FUNCTION
  } Type;
  std::vector<Formal*> args;
  Cmd *body;
  Def(Type type,
      const std::string &name, 
      const std::vector<Formal*> &args, 
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
  Decl(Type type, const std::string &name) :
    Spec(Spec::DECL, name), type(type) {}
};

struct VarDecl : public Decl {
  VarDecl(const std::string &name) :
    Decl(Decl::VAR, name) {}
};

struct ArrayDecl : public Decl {
  std::vector<Expr*> sizes;
  ArrayDecl(const std::string &name,
            const std::vector<Expr*> &sizes) :
    Decl(Decl::ARRAY, name), sizes(sizes) {}
};

struct Abbr : public Spec {
  typedef enum {
    VAL,
    VAR
  } Type;
  Type type;
  Abbr(Type type, const std::string &name) :
    Spec(Spec::ABBR, name), type(type) {}
};

struct ValAbbr : public Abbr {
  Elem *elem;
  ValAbbr(const std::string &name, Elem *elem) :
    Abbr(Abbr::VAL, name), elem(elem) {}
};

struct VarAbbr : public Abbr {
  Expr *expr;
  VarAbbr(const std::string &name, Expr* expr) :
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
  Loop() : Cmd(Cmd::LOOP) {
  }
};

struct Seq : public Cmd {
  Seq() : Cmd(Cmd::SEQ) {
  }
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

// Elements ===================================================================

struct Elem : public Node {
  typedef enum {
    SUBSCRIPT,
    FIELD,
    NAME,
    LITERAL
  } Type;
};

#endif
