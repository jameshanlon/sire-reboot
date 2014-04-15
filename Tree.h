#ifndef TREE_H
#define TREE_H

#include <vector>
#include <string>

class Node;

class Tree {
// The syntax tree base class
public:
  typedef enum {
    t_SERVER, 
    t_PROCESS, 
    t_FUNCTION,
    t_VAL, 
    t_VAR,
    t_ARRAY
  } Type;

  std::vector<Node> specification;
  std::vector<Node> program;
  void print();
};

class Node {
// The tree node base class
public:
  Tree::Type type;
  Node(Tree::Type type) : type(type) {}
};

class Formal {
};

class Command {
};

class Specifier {
};

class Expr {
};

// Program ====================================================================

class Def : public Node {
public:
  std::string name;
  std::vector<Formal*> args;
  std::vector<Command*> body;
  Def(Tree::Type type,
      std::string name, 
      std::vector<Formal*> args, 
      std::vector<Command*> body) : 
    Node(type), 
    name(name),
    args(args), 
    body(body) {}
};

// Specifications =============================================================

// Declarations

class Decl : public Node {
public:
  std::string name;
  Decl(Tree::Type type,
      std::string name) :
    Node(type),
    name(name) {}
};

class ArrayDecl : public Decl {
public:
  std::vector<Expr *> sizes;
  ArrayDecl(std::string name, 
      std::vector<Expr *> sizes) :
    Decl(Tree::t_ARRAY, name),
    sizes(sizes) {}
};

// Abbreviations

class Abbr : public Node {
  //Specifier *spec;
};

class ValAbbr : public Abbr {
};

#endif
