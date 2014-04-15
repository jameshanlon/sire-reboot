#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

struct Node;

struct Tree {
// The syntax tree base struct
  std::vector<Node*> specification;
  std::vector<Node*> program;
  void print();
};

struct Node {
// The tree node base struct
public:
  Tree::Type type;
  Node(Tree::Type type) : type(type) {}
};

// Base Node types
struct Specification;
struct Command;
struct Formal;
struct Expr;
struct Elem;

// Program ====================================================================

struct Def : public Node {
  typedef enum {
    t_SERVER, 
    t_PROCESS, 
    t_FUNCTION,
  } Type;
  std::string name;
  std::vector<Formal*> args;
  std::vector<Command*> body;
  Def(Tree::Type type,
      const std::string &name, 
      const std::vector<Formal*> &args, 
      const std::vector<Command*> &body) : 
    Node(type), 
    name(name),
    args(args), 
    body(body) {}
};

// Specifications =============================================================

struct Specification : public Node {
  typedef enum {
    VAR_DECL,
    ARRAY_DECL,
    VAL_ABBRV,
    VAR_ABBRV,
  } Type;
  std::string name;
  Specification() {}
}

struct VarDecl : public Specification {
  VarDecl(const std::string &name) :
    Node(Specification::VAR_DECL),
    name(name) {}
};

struct ArrayDecl : public Specification {
  std::vector<Expr*> sizes;
  ArrayDecl(const std::string &name, 
            const std::vector<Expr*> &sizes) :
    Specification(Specification::ARRAY_DECL),
    sizes(sizes) {}
};

struct ValAbbrv : public Specification {
  Elem *elem;
  ValAbbrv(const std::string &name,
           Elem *elem) :
    Specification(Specification::VAL_ABBRV),
    elem(elem) {}
};

struct VarAbbrv : public Specification {
  Expr *expr;
  ValAbbrv(const std::string &name,
           Expr* expr) :
    Specification(Specification::VAL_ABBRV),
    expr(expr) {}
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
