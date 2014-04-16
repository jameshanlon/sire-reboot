#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

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
struct Specification;
struct Command;
struct Formal;
struct Expr;
struct Elem;

// Definitions ================================================================

// Specifications =============================================================

struct Spec : public Node {
  typedef enum {
    DEF,
    DECL,
    ABBR
  } Type;
  Type type;
  std::string name;
  Spec(Type type, std::string &name) : 
    type(type), name(name) {}
  virtual void accept(Visitor *v) = 0;
}

struct Def : public Specification {
  typedef enum {
    SERVER,
    PROCESS,
    FUNCTION
  } Type;
  std::vector<Formal*> args;
  Cmd body;
  Def(Tree::Type type,
      const std::string &name, 
      const std::vector<Formal*> &args, 
      const std::vector<Command*> &body) : 
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
    Decl(Specification::VAR_DECL, name) {}
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

struct ValAbbrv : public ABBR {
  Elem *elem;
  ValAbbrv(const std::string &name, Elem *elem) :
    Abbr(Abbr::VAL, name), elem(elem) {}
};

struct VarAbbrv : public ABBR {
  Expr *expr;
  ValAbbrv(const std::string &name, Expr* expr) :
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
    LOOP,
    SEQ,
    PAR
  } Type;
  Type type;
  Cmd(Type type) : type(type) {}
};

struct Skip : public Cmd {
  Skip() : Cmd(SKIP) {}
};

struct Stop : public Cmd {
  Skip() : Cmd(STOP) {}
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
    BINARY
    VALOF
  } Type;
};

// Elements ===================================================================

struct Elem : public Node {
  typedef enum {
    SUBSCRIPT,
    FIELD,
    NAME
    LITERAL
  } Type;
};

#endif
