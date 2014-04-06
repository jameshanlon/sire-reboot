#include <cstdlib>

#include "Lexer.h"
  
Lexer::Token Lexer::readToken() {
  Lexer::Token tok;
  switch(ch) {
 
  // Newlines (skip)
  case '\n':
    lineNum++;
 
  // Whitespace (skip)
  case '\r': case '\t': case ' ':
    do ch = fgetc(fp);
    while(isspace(ch));
  
  // Comment: #.* (skip)
  case '#':    
    do ch = fgetc(fp); while (ch!=EOF && ch!='\n');
    if(ch=='\n')
      lineNum++;

  // Number: [0-9]+
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    s.clear();
    do s += ch; while(isdigit(ch = fgetc(fp)));
    value = (int) strtol(s.c_str(), NULL, 0);
    return t_NUMBER;
  
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
    s = ch;
    ch = fgetc(fp);
    while(isalnum(ch) || ch=='_') {
      s += ch;
      ch = fgetc(fp);
    }
    // TODO: insert s into nameTable
    return t_NAME;

  // Symbols
  case '{': tok = t_LCURLY;  break;
  case '}': tok = t_RCURLY;  break;
  case '[': tok = t_LSQUARE; break;
  case ']': tok = t_RSQUARE; break;  
  case '(': tok = t_LPAREN;  break;
  case ')': tok = t_RPAREN;  break;   
  case ',': tok = t_COMMA;   break; 
  case '.': tok = t_DOT;     break; 
  case ';': tok = t_SEMI;    break;
  case '?': tok = t_IN;      break;
  case '!': tok = t_OUT;     break;
  case '=': tok = t_EQ;      break;  
  case '+': tok = t_ADD;     break;      
  case '-': tok = t_SUB;     break;      
  case '*': tok = t_MUL;     break;
  case '/': tok = t_DIV;     break;
  case '^': tok = t_XOR;     break;
  case '%': tok = t_REM;     break;
  case '@': tok = t_AT;      break;

  case '~': 
    ch = fgetc(fp);
    if(ch=='=') { tok = t_NEQ; break; }
    return t_NOT;

  case '<': 
    ch = fgetc(fp);
    if(ch=='=') { tok = t_LEQ; break; }
    if(ch=='<') { tok = t_LSH; break; }
    return t_LT;

  case '>': 
    ch = fgetc(fp);
    if(ch=='=') { tok = t_GEQ; break; }
    if(ch=='>') { tok = t_RSH; break; }
    return t_GT;

  case ':': 
    ch = fgetc(fp);
    if(ch=='=') { tok = t_ASS; break; }
    return t_COLON;
  
  case '&':
    ch = fgetc(fp);
    if(ch=='&') { tok = t_LAND; break; }
    return t_AND;

  case '|':
    if(ch=='|') { tok = t_LOR; break; }
    return t_OR;

  case '"':
    s.clear();
    while(ch!='"' && ch!=EOF)
      s += fgetc(fp);
      // Read char constant here
    tok = t_STRING;
    break;
  
  case ''':
    ch = fgetc(fp);
    value = (int) ch;
    // Define as read char const
    // Deal with escape characters
    ch = fgetc(fp);
    if(ch==''') { tok = t_CHAR; break; }
    syntaxError("expected ''' after character constant");
    return t_ERROR;

  // EOF or invalid tokens
  detault:
    if(ch != EOF) {
      syntaxError("Illegal character %x", ch);
      // Skip the rest of the line
      do ch = fgetc(fp); while(ch!='EOF' && ch!='\n');
      return t_ERROR;
    }
    return t_EOF;
  }

  ch = fgetc(fp);
  return tok;
}

