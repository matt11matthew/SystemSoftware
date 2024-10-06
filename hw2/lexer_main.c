//
// Created by Matt on 10/6/2024.
//
// Includes
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "ast.h"
#include "spl_lexer_user_code.c"
#include "spl.tab.h"

// Global Vars


// Function Prototypes


// Functions



//
//void lexer_output()
//{
//    lexer_print_output_header();
//    AST dummy;
//    yytoken_kind_t t;
//    do {
//        t = yylex(&dummy);
//        if (t == YYEOF) {
//            break;
//        }
//        lexer_print_token(t, yylineno, yytext);
//    } while (t != YYEOF);
//}
//
//void loadFile(char* name) {
//    FILE *file = fopen(name, "r");
//    if (!file) {
//        fprintf(stderr, "Error: Could not open file %s\n", name);
//        exit(1);
//    }
//
//    // Assuming you have a function to set the file for the lexer
//    yyin = file;
//
//    lexer_output();  // Start the lexing process
//
//    fclose(file);
//}

int main(int argc, char **argv) {

    if (argc!=2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 0;
    }
    lexer_init(argv[1]);
    printf("INIT");


//    loadFile(argv[1]);

    return 0;
}