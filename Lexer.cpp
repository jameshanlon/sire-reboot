#include "Lexer.h"
#include "SymTable.h"
#include "Error.h"

#include <cstdlib>

Lexer Lexer::instance;

void Lexer::init(FILE *p) {
  fp = p;
  declareKeywords();
  readChar();
}

Lexer::Token Lexer::readToken() {
  Lexer::Token tok;
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

  // Number: [0-9]+
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    readNumber();
    return t_NUM;
  
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
    return (Lexer::Token) TAB.lookup(s);

  // Symbols
  case '{': tok = t_LCURLY;  break;
  case '}': tok = t_RCURLY;  break;
  case '[': tok = t_LSQ;     break;
  case ']': tok = t_RSQ;     break;  
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
    readChar();
    if(ch=='=') { tok = t_NEQ; break; }
    return t_NOT;

  case '<': 
    readChar();
    if(ch=='=') { tok = t_LEQ; break; }
    if(ch=='<') { tok = t_LSH; break; }
    return t_LT;

  case '>': 
    readChar();
    if(ch=='=') { tok = t_GEQ; break; }
    if(ch=='>') { tok = t_RSH; break; }
    return t_GT;

  case ':': 
    readChar();
    if(ch=='=') { tok = t_ASS; break; }
    return t_COLON;
  
  case '&':
    readChar();
    if(ch=='&') { tok = t_LAND; break; }
    return t_AND;

  case '|':
    if(ch=='|') { tok = t_LOR; break; }
    return t_OR;

  case '"':
    s.clear();
    while(ch!='"' && ch!=EOF)
      s += readStrCh();
    tok = t_STR;
    break;
  
  case '\'':
    readChar();
    value = (int) ch;
    ch = readStrCh();
    if(ch=='\'') { tok = t_CHAR; break; }
    error("expected ''' after character constant");
    return t_ERROR;

  // EOF or invalid tokens
  default:
    if(ch != EOF) {
      error("illegal character");
      // Skip the rest of the line
      do readChar(); while(ch!=EOF && ch!='\n');
      return t_ERROR;
    }
    return t_EOF;
  }

  readChar();
  return tok;
}

void Lexer::readChar() {
  ch = fgetc(fp);
  chBuf[++chCount & (BUF_SIZE-1)] = ch;
}

void Lexer::printChBuf() {
  printf("\n...");
  for(int i=chCount+1; i<chCount+BUF_SIZE; i++) {
    char c = chBuf[i & (BUF_SIZE-1)];
    if(c)
      printf("%c", c);
  }
  printf("\n");
}

void Lexer::skipLine() {
  do readChar(); while (ch!=EOF && ch!='\n');
  if(ch=='\n')
    lineNum++;
}

void Lexer::readNumber() {
  s.clear();
  do {
    s += ch; 
    readChar(); 
  } while('0'<=ch && ch<='9');
  value = (int) strtol(s.c_str(), NULL, 0);
}

void Lexer::readName() {
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

char Lexer::readStrCh() {
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

void Lexer::declare(const char *keyword, Lexer::Token t) {
  TAB.insert(std::string(keyword), t);
}

void Lexer::declareKeywords() {
  declare("accept",    Lexer::t_ACCEPT); 
  declare("alt",       Lexer::t_ALT);  
  declare("call",      Lexer::t_CALL);
  declare("case",      Lexer::t_CASE);
  declare("chan",      Lexer::t_CHAN);
  declare("connect",   Lexer::t_CONNECT); 
  declare("do",        Lexer::t_DO);    
  declare("else",      Lexer::t_ELSE);  
  declare("false",     Lexer::t_FALSE);
  declare("final",     Lexer::t_FINAL);   
  declare("for",       Lexer::t_FOR);    
  declare("from",      Lexer::t_FROM);
  declare("function",  Lexer::t_FUNC);
  declare("if",        Lexer::t_IF);
  declare("inherits",  Lexer::t_INHRT); 
  declare("initial",   Lexer::t_INIT);
  declare("interface", Lexer::t_INTF);
  declare("is",        Lexer::t_IS);     
  declare("on",        Lexer::t_ON);     
  declare("par",       Lexer::t_PAR);
  declare("process",   Lexer::t_PROC);
  declare("result",    Lexer::t_RES);  
  declare("seq",       Lexer::t_SEQ);  
  declare("server",    Lexer::t_SERV);  
  declare("skip",      Lexer::t_SKIP);
  declare("step",      Lexer::t_STEP);   
  declare("stop",      Lexer::t_STOP);    
  declare("then",      Lexer::t_THEN);    
  declare("to",        Lexer::t_TO);
  declare("true",      Lexer::t_TRUE);
  declare("until",     Lexer::t_UNTIL);
  declare("val",       Lexer::t_VAL);   
  declare("valof",     Lexer::t_VALOF);   
  declare("var",       Lexer::t_VAR);
  declare("while",     Lexer::t_WHILE);                         
}

void Lexer::error(const char *msg) {
  printf("Error near line %d: %s\n", lineNum, msg);
  printChBuf();
  ERR.record();
  // Skip up to a safer point
  Lexer::Token tok = readToken();
  while(tok != t_EOF 
      || tok != t_SEMI 
      || tok != t_AND 
      || tok != t_RCURLY 
      || tok != t_LCURLY)
    tok = readToken();
}

const char *Lexer::tokenStr(Lexer::Token t) {
  switch(t) { 
    default:        return "unknown";
    case t_ERROR:   return "Error";
    // Literals
    case t_NUM:     return "Number";  case t_NAME:    return "Name";
    // Symbols
    case t_LCURLY:  return "Lcurly";  case t_RCURLY:  return "Rcurly"; 
    case t_LSQ:     return "Lsquare"; case t_RSQ:     return "Rsquare";
    case t_LPAREN:  return "Lparen";  case t_RPAREN:  return "Rparen"; 
    case t_COMMA:   return "Comma";   case t_DOT:     return "Dot";
    case t_SEMI:    return "Semi";    case t_IN:      return "In";     
    case t_OUT:     return "Out";     case t_EQ:      return "Eq";
    case t_ADD:     return "Add";     case t_SUB:     return "Sub";    
    case t_MUL:     return "Mul";     case t_DIV:     return "Div";
    case t_XOR:     return "Xor";     case t_REM:     return "Rem";     
    case t_AT:      return "At";      case t_NEQ:     return "Neq";
    case t_NOT:     return "Not";     case t_LEQ:     return "Leq";     
    case t_LSH:     return "Lsh";     case t_LT:      return "Lt";
    case t_GEQ:     return "Geq";     case t_RSH:     return "Rsh";      
    case t_GT:      return "Gt";      case t_ASS:     return "Ass";
    case t_COLON:   return "Colon";   case t_LAND:    return "Land";     
    case t_AND:     return "And";     case t_LOR:     return "Lor";
    case t_OR:      return "Or";      case t_STR:     return "String";
    case t_CHAR:    return "Char";
    // Keywords
    case t_ACCEPT:  return "Accept";  case t_ALT:     return "Alt"; 
    case t_CALL:    return "Call";    case t_CHAN:    return "Chan";
    case t_CONNECT: return "Connet";  case t_DO:      return "Do";   
    case t_ELSE:    return "Else";    case t_FALSE:   return "False";
    case t_FINAL:   return "Final";   case t_FOR:     return "For";
    case t_FROM:    return "From";    case t_FUNC:    return "Function";
    case t_IF:      return "If";      case t_INHRT:   return "Inherits";
    case t_INIT:    return "Initial"; case t_INTF:    return "Interface";
    case t_IS:      return "Is";      case t_ON:      return "On"; 
    case t_PAR:     return "Par";     case t_PROC:    return "Process";
    case t_RES:     return "Result";  case t_SEQ:     return "Seq";
    case t_SERV:    return "Server";  case t_SKIP:    return "Skip";
    case t_STEP:    return "Step";    case t_STOP:    return "Stop";
    case t_THEN:    return "Then";    case t_TO:      return "To";
    case t_TRUE:    return "True";    case t_VAL:     return "Val";     
    case t_VALOF:   return "Valof";   case t_VAR:     return "Var";
    case t_WHILE:   return "While"; 
  }
}
