#ifndef LEXER_H
#define LEXER_H

#include <string>
#include "stdio.h"

#define BUF_SIZE 64
#define LEX Lexer::get()

class Lexer {

// The lexical analyser

public:
  typedef enum {
    t_EOF=1,
    t_ERROR,
    // Literals
    t_NUMBER,
    t_NAME,
    // Symbols
    t_LCURLY,  t_RCURLY,   t_LSQUARE, t_RSQUARE,
    t_LPAREN,  t_RPAREN,   t_COMMA,   t_DOT,
    t_SEMI,    t_IN,       t_OUT,     t_EQ,  
    t_ADD,     t_SUB,      t_MUL,     t_DIV,
    t_XOR,     t_REM,      t_AT,      t_NEQ,
    t_NOT,     t_LEQ,      t_LSH,     t_LT,
    t_GEQ,     t_RSH,      t_GT,      t_ASS,
    t_COLON,   t_LAND,     t_AND,     t_LOR,
    t_OR,      t_STRING,   t_CHAR,    
    // Keywords
    t_ACCEPT,  t_ALT,      t_CALL,    t_CHANEND,
    t_CONNECT, t_DO,       t_ELSE,    t_FALSE,
    t_FINAL,   t_FOR,      t_FROM,    t_FUNCTION,
    t_IF,      t_INHERITS, t_INITIAL, t_INTERFACE,
    t_IS,      t_ON,       t_PAR,     t_PROCESS,
    t_RESULT,  t_SEQ,      t_SERVER,  t_SKIP,
    t_STEP,    t_STOP,     t_THEN,    t_TO,
    t_TRUE,    t_VAL,      t_VALOF,   t_VAR,
    t_WHILE                            
  } Token;

  static Lexer instance;
  static Lexer &get() { return instance; }

  FILE* fp;
  int lineNum;
  int linePos;
  int value;
  std::string s;
  char ch;
  int chCount;
  char chBuf[BUF_SIZE];

  Lexer() : lineNum(1), chCount(0) {
    for(int i=0; i<BUF_SIZE; i++) 
      chBuf[i] = 0;
  }
  ~Lexer() {}
  void init();
  void readChar();
  Token readToken();
  void declareKeywords();
  const char *tokenStr(Lexer::Token t);
  
private:
  void readNumber();
  void readName();
  char readStrCh();
  void printChBuf();
  void skipLine();
  void error(const char *msg);
  void declare(const char *keyword, Token);
};

#endif
