#include "Error.h"
#include "Table.h"
#include "Lex.h"
#include "Syn.h"

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <string.h>
#include <unistd.h>

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

    // Print tokens from lexer
    if (optPrintTokens) {
      Lex::Token t;
      while ((t = LEX.readToken()) != Lex::tEOF)
        LEX.printToken(t);
    }
    else {
      Tree *tree = SYN.formTree();
      tree->print();
      //TRN.translateTree();
    }
  }
  catch(FatalError &e) {
    fprintf(stderr, "Error: %s\n", e.msg());
    if (fp != nullptr) 
      fclose(fp);
    return 1;
  }

  fclose(fp);
  return 0;
}

