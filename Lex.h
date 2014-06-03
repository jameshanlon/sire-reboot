#ifndef LEXER_H
#define LEXER_H

#include "stdio.h"
#include <string>

#define BUF_SIZE 64
#define LEX Lex::get()

class Lex {

// The lexical analyser

public:
  typedef enum {
    t_EOF=1,
    t_ERROR,
    // Literals
    t_NUM,     t_NAME,
    // Symbols
    t_LCURLY,  t_RCURLY, t_LSQ,   t_RSQ,
    t_LPAREN,  t_RPAREN, t_COMMA, t_DOT,
    t_SEMI,    t_IN,     t_OUT,   t_EQ,  
    t_ADD,     t_SUB,    t_MUL,   t_DIV,
    t_XOR,     t_REM,    t_AT,    t_NEQ,
    t_NOT,     t_LEQ,    t_LSH,   t_LT,
    t_GEQ,     t_RSH,    t_GT,    t_ASS,
    t_COLON,   t_LAND,   t_AND,   t_LOR,
    t_OR,      t_STR,    t_CHAR,  
    // Keywords
    t_ACCEPT,  t_ALT,    t_CALL,  t_CHAN,
    t_CONNECT, t_DO,     t_ELSE,  t_FALSE,
    t_FINAL,   t_FOR,    t_FROM,  t_FUNC,
    t_IF,      t_INHRT,  t_INIT,  t_INTF,
    t_IS,      t_ON,     t_PAR,   t_PROC,
    t_RES,     t_SEQ,    t_SERV,  t_SKIP,
    t_STEP,    t_STOP,   t_THEN,  t_TO,
    t_TRUE,    t_VAL,    t_VALOF, t_VAR,
    t_WHILE,   t_UNTIL,  t_CASE,  t_TEST                      
  } Token;

  static Lex instance;
  static Lex &get() { return instance; }

  FILE* fp;
  int lineNum;
  int linePos;
  int value;
  std::string s;
  char ch;
  int chCount;
  char chBuf[BUF_SIZE];
  bool nlPending;

  Lex() : lineNum(1), chCount(0) {
    for(int i=0; i<BUF_SIZE; i++) 
      chBuf[i] = 0;
  }
  ~Lex() {}
  void init(FILE *p);
  void error(const char *msg);
  void readChar();
  Token readToken();
  void declareKeywords();
  const char *tokStr(Lex::Token t);
  
private:
  void readNumber();
  void readName();
  char readStrCh();
  void printChBuf();
  void skipLine();
  void declare(const char *keyword, Token);
};

#endif
