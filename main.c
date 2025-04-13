#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

extern FILE* yyin;
extern int yyparse();

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", argv[1]);
        return EXIT_FAILURE;
    }

    yyin = input_file;
    printf("Parsing %s...\n", argv[1]);
    
    int result = yyparse();
    
    fclose(input_file);
    
    if (result == 0) {
        printf("Parsing completed successfully.\n");
        return EXIT_SUCCESS;
    } else {
        printf("Parsing failed.\n");
        return EXIT_FAILURE;
    }
}