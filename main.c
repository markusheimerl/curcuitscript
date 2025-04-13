#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcb_model.h"
#include "gerber_export.h"

extern FILE* yyin;
extern int yyparse();
extern Board* current_board;

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file> [output_directory]\n", argv[0]);
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
        
        // Define output directory - default to "./output" if not specified
        const char* output_dir = argc > 2 ? argv[2] : "./output";
        
        // Generate Gerber and drill files
        generate_gerber_files(current_board, output_dir);
        generate_drill_files(current_board, output_dir);
        
        // Free allocated memory
        free_board(current_board);
        
        return EXIT_SUCCESS;
    } else {
        printf("Parsing failed.\n");
        return EXIT_FAILURE;
    }
}