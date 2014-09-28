#include "Lex.h"
#include "Table.h"
#include "Error.h"

#include <stdlib.h>

Lex Lex::instance;

void Lex::init(FILE *p) {
  fp = p;
  declareKeywords();
  readChar();
}

Lex::Token Lex::readToken() {
  Lex::Token tok;
  switch(ch) {

  // Newlines (skip)
  case '\n':
    lineNum++;
    readChar();
    return readToken();

  // Whitespace (skip)
  case '\r': case '\t': case ' ':
    do readChar(); while(ch=='\r' || ch=='\t' || ch==' ');
    return readToken();

  // Comment: #.* (skip)
  case '#':
    do readChar(); while (ch!=EOF && ch!='\n');
    if(ch=='\n')
      lineNum++;
    readChar();
    return readToken();

  // Number literal: [0-9]+
  case '1': case '2': case '3': case '4': case '5':
  case '6': case '7': case '8': case '9':
    readDecInt();
    return tDECINT;

  // Number literals: hex, octal and binary
  case '0':
    readChar();
    if (ch=='x') {
      readHexInt();
      tok = tHEXINT;
      break;
    }
    if (ch=='o') {
      readOctInt();
      tok = tOCTINT;
      break;
    }
    if (ch=='b') {
      readBinInt();
      tok = tBININT;
      break;
    }

  // Name: [a-zA-Z][a-zA-Z0-9]*
  case 'a': case 'b': case 'c': case 'd': case 'e':
  case 'f': case 'g': case 'h': case 'i': case 'j':
  case 'k': case 'l': case 'm': case 'n': case 'o':
  case 'p': case 'q': case 'r': case 's': case 't':
  case 'u': case 'v': case 'w': case 'x': case 'y':
  case 'z':
  case 'A': case 'B': case 'C': case 'D': case 'E':
  case 'F': case 'G': case 'H': case 'I': case 'J':
  case 'K': case 'L': case 'M': case 'N': case 'O':
  case 'P': case 'Q': case 'R': case 'S': case 'T':
  case 'U': case 'V': case 'W': case 'X': case 'Y':
  case 'Z':
    readName();
    return (Lex::Token) TAB.lookup(s);

  // Symbols
  case '{': tok = tLCURLY;  break;
  case '}': tok = tRCURLY;  break;
  case '[': tok = tLSQ;     break;
  case ']': tok = tRSQ;     break;
  case '(': tok = tLPAREN;  break;
  case ')': tok = tRPAREN;  break;
  case ',': tok = tCOMMA;   break;
  case '.': tok = tDOT;     break;
  case ';': tok = tSEMI;    break;
  case '?': tok = tIN;      break;
  case '!': tok = tOUT;     break;
  case '=': tok = tEQ;      break;
  case '+': tok = tADD;     break;
  case '-': tok = tSUB;     break;
  case '*': tok = tMUL;     break;
  case '/': tok = tDIV;     break;
  case '^': tok = tXOR;     break;
  case '%': tok = tREM;     break;
  case '@': tok = tAT;      break;

  case '~':
    readChar();
    if(ch=='=') { tok = tNEQ; break; }
    return tNOT;

  case '<':
    readChar();
    if(ch=='=') { tok = tLEQ; break; }
    if(ch=='<') { tok = tLSH; break; }
    return tLT;

  case '>':
    readChar();
    if(ch=='=') { tok = tGEQ; break; }
    if(ch=='>') { tok = tRSH; break; }
    return tGT;

  case ':':
    readChar();
    if(ch=='=') { tok = tASS; break; }
    return tCOLON;

  case '&':
    readChar();
    if(ch=='&') { tok = tLAND; break; }
    return tAND;

  case '|':
    if(ch=='|') { tok = tLOR; break; }
    return tOR;

  case '"':
    s.clear();
    while(ch!='"' && ch!=EOF)
      s += readStrCh();
    tok = tSTR;
    break;

  case '\'':
    readChar();
    value = (int) ch;
    ch = readStrCh();
    if(ch=='\'') { tok = tCHAR; break; }
    error("expected ''' after character constant");
    return tERROR;

  // EOF or invalid tokens
  default:
    if(ch != EOF) {
      error("illegal character");
      // Skip the rest of the line
      do readChar(); while(ch!=EOF && ch!='\n');
      return tERROR;
    }
    return tEOF;
  }

  readChar();
  return tok;
}

void Lex::printToken(Token t) {
  printf("token %3d %s ", (int) t, tokStr(t));
  if(t == Lex::tNAME)   printf("%s", s.c_str());
  if(t == Lex::tSTR)    printf("%s", s.c_str());
  if(t == Lex::tDECINT) printf("%d", value);
  if(t == Lex::tHEXINT) printf("%x", value);
  if(t == Lex::tOCTINT) printf("%o", value);
  if(t == Lex::tBININT) {
    int val = value;
    char s[BUF_SIZE+1];
    char *p = s + BUF_SIZE;
    do { *--p = '0' + (val & 1); } while (val >>= 1);
    printf("%s", s);
  }
  printf("\n");
}

void Lex::readChar() {
  ch = fgetc(fp);
  chBuf[++chCount & (BUF_SIZE-1)] = ch;
}

void Lex::printChBuf() {
  printf("\n...");
  for(int i=chCount+1; i<chCount+BUF_SIZE; i++) {
    char c = chBuf[i & (BUF_SIZE-1)];
    if(c)
      printf("%c", c);
  }
  printf("\n");
}

void Lex::skipLine() {
  do readChar(); while (ch!=EOF && ch!='\n');
  if(ch=='\n')
    lineNum++;
}

void Lex::readDecInt() {
  s.clear();
  do {
    s += ch;
    readChar();
  } while('0'<=ch && ch<='9');
  value = (int) strtol(s.c_str(), nullptr, 0);
}

void Lex::readHexInt() {
  s.clear();
  do {
    s += ch;
    readChar();
  } while(('0'<=ch && ch<='9')
       || ('a'<=ch && ch<='z')
       || ('A'<=ch && ch<='Z'));
  value = (int) strtol(s.c_str(), nullptr, 16);
}

void Lex::readOctInt() {
  s.clear();
  do {
    s += ch;
    readChar();
  } while('0'<=ch && ch<='7');
  value = (int) strtol(s.c_str(), nullptr, 8);
}

void Lex::readBinInt() {
  s.clear();
  do {
    s += ch;
    readChar();
  } while('0'<=ch && ch<='1');
  value = (int) strtol(s.c_str(), nullptr, 2);
}

void Lex::readName() {
  s.clear();
  s = ch;
  readChar();
  while(('a'<=ch && ch<='z') ||
        ('A'<=ch && ch<='Z') ||
        ('0'<=ch && ch<='9') ||  ch=='_') {
    s += ch;
    readChar();
  }
}

char Lex::readStrCh() {
  char res = ch;
  if(ch == '\n') {
    lineNum++;
    error("expected ''' after character constant");
  }
  if(ch == '\\') {
    readChar();
    switch(ch) {
      default: error("bad string or character constant");
      case '\\': case '\'': case '"': res = ch;   break;
      case 't':  case 'T':            res = '\t'; break;
      case 'r':  case 'R':            res = '\r'; break;
      case 'n':  case 'N':            res = '\n'; break;
    }
  }
  readChar();
  return res;
}

void Lex::declare(const char *keyword, Lex::Token t) {
  TAB.insert(std::string(keyword), t);
}

void Lex::declareKeywords() {
  declare("accept",    Lex::tACCEPT);
  declare("alt",       Lex::tALT);
  declare("call",      Lex::tCALL);
  declare("case",      Lex::tCASE);
  declare("chan",      Lex::tCHAN);
  declare("connect",   Lex::tCONNECT);
  declare("do",        Lex::tDO);
  declare("else",      Lex::tELSE);
  declare("false",     Lex::tFALSE);
  declare("final",     Lex::tFINAL);
  declare("for",       Lex::tFOR);
  declare("from",      Lex::tFROM);
  declare("function",  Lex::tFUNCTION);
  declare("if",        Lex::tIF);
  declare("inherits",  Lex::tINHRT);
  declare("initial",   Lex::tINIT);
  declare("interface", Lex::tINTF);
  declare("is",        Lex::tIS);
  declare("on",        Lex::tON);
  declare("par",       Lex::tPAR);
  declare("process",   Lex::tPROCESS);
  declare("result",    Lex::tRESULT);
  declare("seq",       Lex::tSEQ);
  declare("server",    Lex::tSERVER);
  declare("skip",      Lex::tSKIP);
  declare("step",      Lex::tSTEP);
  declare("stop",      Lex::tSTOP);
  declare("test",      Lex::tTEST);
  declare("then",      Lex::tTHEN);
  declare("to",        Lex::tTO);
  declare("true",      Lex::tTRUE);
  declare("until",     Lex::tUNTIL);
  declare("val",       Lex::tVAL);
  declare("valof",     Lex::tVALOF);
  declare("var",       Lex::tVAR);
  declare("while",     Lex::tWHILE);
}

void Lex::error(const char *msg) {
  printf("Error near line %d: %s\n", lineNum, msg);
  printChBuf();
  ERR.record();
  // Skip up to a safer point
  // TODO: fix this behaviour
  Lex::Token t = readToken();
  printf("recover: ");
  LEX.printToken(t);
  //while (t != tEOF
  //    || t != tSEMI
  //    || t != tAND
  //    || t != tRCURLY
  //    || t != tLCURLY) {
  //  t = readToken();
  //  printf("recover: ");
  //  LEX.printToken(t);
  //}
}

const char *Lex::tokStr(Lex::Token t) {
  switch(t) {
    default:       return "unknown";
    case tERROR:   return "error";
    case tEOF:     return "EOF";
    // Literals
    case tDECINT:  return "decimal"; case tHEXINT:   return "hexdecimal";
    case tOCTINT:  return "octal";   case tBININT:   return "binary";
    case tNAME:    return "name";
    // Symbols
    case tLCURLY:  return "{";       case tRCURLY:   return "}";
    case tLSQ:     return "[";       case tRSQ:      return "]";
    case tLPAREN:  return "(";       case tRPAREN:   return ")";
    case tCOMMA:   return ",";       case tDOT:      return ".";
    case tSEMI:    return ";";       case tIN:       return "?";
    case tOUT:     return "!";       case tEQ:       return "=";
    case tADD:     return "+";       case tSUB:      return "-";
    case tMUL:     return "*";       case tDIV:      return "/";
    case tXOR:     return "^";       case tREM:      return "%";
    case tAT:      return "at";      case tNEQ:      return "~=";
    case tNOT:     return "~";       case tLEQ:      return "<=";
    case tLSH:     return "<<";      case tLT:       return "<";
    case tGEQ:     return ">=";      case tRSH:      return ">>";
    case tGT:      return ">";       case tASS:      return ":=";
    case tCOLON:   return ":";       case tLAND:     return "&&";
    case tAND:     return "&";       case tLOR:      return "||";
    case tOR:      return "|";       case tSTR:      return "string";
    case tCHAR:    return "char";
    // Keywords
    case tACCEPT:  return "accept";  case tALT:      return "alt";
    case tCALL:    return "call";    case tCHAN:     return "chan";
    case tCONNECT: return "connect"; case tDO:       return "do";
    case tELSE:    return "else";    case tFALSE:    return "false";
    case tFINAL:   return "final";   case tFOR:      return "for";
    case tFROM:    return "from";    case tFUNCTION: return "function";
    case tIF:      return "if";      case tINHRT:    return "inherits";
    case tINIT:    return "initial"; case tINTF:     return "interface";
    case tIS:      return "is";      case tON:       return "on";
    case tPAR:     return "par";     case tPROCESS:  return "process";
    case tRESULT:  return "result";  case tSEQ:      return "seq";
    case tSERVER:  return "server";  case tSKIP:     return "skip";
    case tSTEP:    return "step";    case tSTOP:     return "stop";
    case tTEST:    return "test";    case tTHEN:     return "then";
    case tTO:      return "to";      case tTRUE:     return "true";
    case tUNTIL:   return "until";   case tVAL:      return "val";
    case tVALOF:   return "valof";   case tVAR:      return "var";
    case tWHILE:   return "while";
  }
}
