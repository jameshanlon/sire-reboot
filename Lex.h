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
    tEOF=1,
    tERROR,
    // Literals
    tNUM,     tNAME,
    // Symbols
    tLCURLY,  tRCURLY, tLSQ,    tRSQ,
    tLPAREN,  tRPAREN, tCOMMA,  tDOT,
    tSEMI,    tIN,     tOUT,    tEQ,  
    tADD,     tSUB,    tMUL,    tDIV,
    tXOR,     tREM,    tAT,     tNEQ,
    tNOT,     tLEQ,    tLSH,    tLT,
    tGEQ,     tRSH,    tGT,     tASS,
    tCOLON,   tLAND,   tAND,    tLOR,
    tOR,      tSTR,    tCHAR,   
    // Keywords                
    tACCEPT,  tALT,    tCALL,   tCHAN,
    tCONNECT, tDO,     tELSE,   tFALSE,
    tFINAL,   tFOR,    tFROM,   tFUNCTION,
    tIF,      tINHRT,  tINIT,   tINTF,
    tIS,      tON,     tPAR,    tPROCESS,
    tRES,     tSEQ,    tSERVER, tSKIP,
    tSTEP,    tSTOP,   tTHEN,   tTO,
    tTRUE,    tVAL,    tVALOF,  tVAR,
    tWHILE,   tUNTIL,  tCASE,   tTEST                      
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
