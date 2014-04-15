#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "Error.h"
#include "SymTable.h"
#include "Lexer.h"
#include "Parser.h"

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

int main(int argc, char *argv[]) {
  bool optPrintHelp = false;
  bool optPrintTree = false;
  std::string filename;
  FILE *fp;

  // Parse arguments
  for(int i=1; i<argc; i++) {
    if     (!strcmp(argv[i], "-h")) optPrintHelp = true;
    else if(!strcmp(argv[i], "-p")) optPrintTree = true;
    else                           filename = argv[i];
  }

  // Print help
  if(optPrintHelp) {
    printf("Usage: %s [options] <input>\n\n", argv[0]);
    printf("Options:\n");
    printf("  -h display usage and options\n");
    printf("  -p print the parse tree\n");
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

    // Lexer debug
    Lexer::Token t = LEX.readToken();
    while (t != Lexer::t_EOF) {
      printf("%3d %s ", (int) t, LEX.tokenStr(t));
      if(t == Lexer::t_NUMBER) printf("%d", LEX.value);
      if(t == Lexer::t_NAME)   printf("%s", LEX.s.c_str());
      if(t == Lexer::t_STRING) printf("%s", LEX.s.c_str());
      printf("\n");
      t = LEX.readToken();
    }

    //Tree tree = SYN.formTree();
    //TRN.translateTree();
  }
  catch(FatalError &e) {
    fprintf(stderr, "Error: %s\n", e.msg());
    fclose(fp);
    return 1; 
  }
  
  fclose(fp);
  return 0;
}

