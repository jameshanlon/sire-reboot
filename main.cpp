#include "Error.h"
#include "Table.h"
#include "Lex.h"
#include "Syn.h"

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <string.h>
#include <unistd.h>

#define WORD_SIZE 32

void interpreterLoop() {
//  fprintf(stderr, "> ");
//  syn.getNextToken();
//  while (true) {
//    fprintf(stderr, "> ");
//    switch(curTok) {
//    case t_EOF:     fprintf(stderr, "\n"); return;
//    case t_PROCESS: handleDef();  break;
//    case t_SERVER:  handleDef();  break;
//    case t_VAL:     handleDecl(); break;
//    case t_VAR:     handleDecl(); break;
//    case t_NAME:    handleDecl(); break;
//    default:        handleCmd();  break;
//    }
//  }
}

void printHelp() {
  printf("Usage: sire [options] <input>\n\n");
  printf("Options:\n");
  printf("  -h display usage and options\n");
  printf("  -l print tokenisation only\n");
  printf("  -p print the parse tree\n");
}

int main(int argc, char *argv[]) {
  bool optPrintHelp = false;
  bool optPrintTree = false;
  bool optPrintTokens = false;
  std::string filename;
  FILE *fp;

  // Parse arguments
  for(int i=1; i<argc; i++) {
    if (argv[i][0] == '-') {
      if     (!strcmp(argv[i], "-h")) optPrintHelp = true;
      else if(!strcmp(argv[i], "-l")) optPrintTokens = true;
      else if(!strcmp(argv[i], "-p")) optPrintTree = true;
      else {
        fprintf(stderr, "Invalid argument.\n");
        return 0;
      }
    }
    else {
      filename = argv[i];
    }
  }

  // Print help
  if(optPrintHelp) {
    printHelp();
    return 0;
  }

  try {
    // Open the input file
    fp = filename.empty() ? stdin : fopen(filename.c_str(), "r");
    if(fp == NULL)
      throw FatalError("Could not open the input file");

    TAB.init();
    LEX.init(fp);
    SYN.init();

    // Start interactive mode if not file or pipe
    if(filename.empty() && isatty(fileno(stdin))) {
      //interpreterLoop();
      return 0;
    }

    // Lex debug
    if (optPrintTokens) {
      Lex::Token t = LEX.readToken();
      while (t != Lex::tEOF) {
        printf("%3d %s ", (int) t, LEX.tokStr(t));
        if(t == Lex::tNAME)   printf("%s", LEX.s.c_str());
        if(t == Lex::tSTR)    printf("%s", LEX.s.c_str());
        if(t == Lex::tDECINT) printf("%d", LEX.value);
        if(t == Lex::tHEXINT) printf("%x", LEX.value);
        if(t == Lex::tOCTINT) printf("%o", LEX.value);
        if(t == Lex::tBININT) {
          int val = LEX.value;
          char s[WORD_SIZE+1];
          char *p = s + WORD_SIZE;
          do { *--p = '0' + (val & 1); } while (val >>= 1);
          printf("%s", s);
        }
        printf("\n");
        t = LEX.readToken();
      }
    }
    else {
      Tree *tree = SYN.formTree();
      //TRN.translateTree();
    }
  }
  catch(FatalError &e) {
    fprintf(stderr, "Error: %s\n", e.msg());
    fclose(fp);
    return 1;
  }

  fclose(fp);
  return 0;
}

