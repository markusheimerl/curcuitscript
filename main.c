#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "pcb_model.h"

extern FILE* yyin;
extern int yyparse();
extern Board* current_board;

// Gerber export functions
static void ensure_directory_exists(const char* dir) {
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
#ifdef _WIN32
        mkdir(dir);
#else
        mkdir(dir, 0700);
#endif
    }
}

static void write_gerber_header(FILE* fp, const char* type) {
    fprintf(fp, "%%FSLAX46Y46*%%\n");
    fprintf(fp, "%%MOMM*%%\n");
    fprintf(fp, "%%TF.FileFunction,%s*%%\n", type);
    fprintf(fp, "%%TF.Part,Single*%%\n");
    fprintf(fp, "%%TF.CreationDate,20230101T120000*%%\n");
    fprintf(fp, "%%TF.GenerationSoftware,CircuitScript,1.0*%%\n");
    fprintf(fp, "%%MOIN*%%\n");
    fprintf(fp, "%%ADD10C,0.010*%%\n");
    fprintf(fp, "%%MOMM*%%\n");
}

static void write_gerber_footer(FILE* fp) {
    fprintf(fp, "M02*\n");
}

static int generate_outline(Board* board, const char* output_dir) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s-Edge_Cuts.gbr", 
             output_dir, board->name ? board->name : "board");
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create outline Gerber file: %s\n", filename);
        return -1;
    }
    
    write_gerber_header(fp, "Profile,NP");
    
    fprintf(fp, "G01*\n");
    fprintf(fp, "D10*\n");
    
    int x1 = 0;
    int y1 = 0;
    int x2 = (int)(board->width * 1000000);
    int y2 = (int)(board->height * 1000000);
    
    fprintf(fp, "X%dY%dD02*\n", x1, y1);
    fprintf(fp, "X%dY%dD01*\n", x2, y1);
    fprintf(fp, "X%dY%dD01*\n", x2, y2);
    fprintf(fp, "X%dY%dD01*\n", x1, y2);
    fprintf(fp, "X%dY%dD01*\n", x1, y1);
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated outline Gerber: %s\n", filename);
    return 0;
}

static int generate_copper_layer(Board* board, const char* output_dir, bool top_layer) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s-%s.gbr", 
             output_dir, board->name ? board->name : "board",
             top_layer ? "F_Cu" : "B_Cu");
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create copper layer Gerber file: %s\n", filename);
        return -1;
    }
    
    write_gerber_header(fp, top_layer ? "Copper,L1,Top" : "Copper,L2,Bot");
    
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        if (placement->top_side != top_layer) continue;
        
        Component* component = NULL;
        for (int j = 0; j < board->component_count; j++) {
            if (strcmp(board->components[j].name, placement->component_name) == 0) {
                component = &board->components[j];
                break;
            }
        }
        
        if (!component) {
            fprintf(stderr, "Warning: Component %s not found for placement %s\n", 
                   placement->component_name, placement->ref_designator);
            continue;
        }
        
        for (int j = 0; j < component->pin_count; j++) {
            Pin* pin = &component->pins[j];
            
            double pin_x = placement->position.x + j * 2.54;
            double pin_y = placement->position.y;
            
            int x = (int)(pin_x * 1000000);
            int y = (int)(pin_y * 1000000);
            
            fprintf(fp, "X%dY%dD03*\n", x, y);
        }
    }
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated copper layer Gerber: %s\n", filename);
    return 0;
}

static int generate_silkscreen(Board* board, const char* output_dir, bool top_layer) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s-%s.gbr", 
             output_dir, board->name ? board->name : "board",
             top_layer ? "F_Silkscreen" : "B_Silkscreen");
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create silkscreen Gerber file: %s\n", filename);
        return -1;
    }
    
    write_gerber_header(fp, top_layer ? "Legend,Top" : "Legend,Bot");
    
    fprintf(fp, "G01*\n");
    fprintf(fp, "D10*\n");
    
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        if (placement->top_side != top_layer) continue;
        
        int x1 = (int)((placement->position.x - 5) * 1000000);
        int y1 = (int)((placement->position.y - 2.5) * 1000000);
        int x2 = (int)((placement->position.x + 5) * 1000000);
        int y2 = (int)((placement->position.y + 2.5) * 1000000);
        
        fprintf(fp, "X%dY%dD02*\n", x1, y1);
        fprintf(fp, "X%dY%dD01*\n", x2, y1);
        fprintf(fp, "X%dY%dD01*\n", x2, y2);
        fprintf(fp, "X%dY%dD01*\n", x1, y2);
        fprintf(fp, "X%dY%dD01*\n", x1, y1);
        
        int text_x = (int)(placement->position.x * 1000000);
        int text_y = (int)(placement->position.y * 1000000);
        fprintf(fp, "X%dY%dD02*\n", text_x, text_y);
    }
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated silkscreen Gerber: %s\n", filename);
    return 0;
}

static int generate_excellon(Board* board, const char* output_dir) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.drl", 
             output_dir, board->name ? board->name : "board");
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create Excellon drill file: %s\n", filename);
        return -1;
    }
    
    fprintf(fp, "M48\n");
    fprintf(fp, "METRIC,TZ\n");
    fprintf(fp, "T1C1.000\n");
    fprintf(fp, "%%\n");
    fprintf(fp, "G90\n");
    fprintf(fp, "G05\n");
    fprintf(fp, "T1\n");
    
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        Component* component = NULL;
        for (int j = 0; j < board->component_count; j++) {
            if (strcmp(board->components[j].name, placement->component_name) == 0) {
                component = &board->components[j];
                break;
            }
        }
        
        if (!component) continue;
        
        for (int j = 0; j < component->pin_count; j++) {
            double pin_x = placement->position.x + j * 2.54;
            double pin_y = placement->position.y;
            
            fprintf(fp, "X%.3fY%.3f\n", pin_x, pin_y);
        }
    }
    
    fprintf(fp, "T0\n");
    fprintf(fp, "M30\n");
    
    fclose(fp);
    
    printf("Generated Excellon drill file: %s\n", filename);
    return 0;
}

int generate_gerber_files(Board* board, const char* output_dir) {
    if (!board) {
        fprintf(stderr, "Error: No board data to generate Gerber files\n");
        return -1;
    }
    
    ensure_directory_exists(output_dir);
    
    generate_outline(board, output_dir);
    generate_copper_layer(board, output_dir, true);
    
    if (board->layers >= 2) {
        generate_copper_layer(board, output_dir, false);
    }
    
    generate_silkscreen(board, output_dir, true);
    
    if (board->layers >= 2) {
        generate_silkscreen(board, output_dir, false);
    }
    
    printf("Gerber files generation complete\n");
    return 0;
}

int generate_drill_files(Board* board, const char* output_dir) {
    if (!board) {
        fprintf(stderr, "Error: No board data to generate drill files\n");
        return -1;
    }
    
    ensure_directory_exists(output_dir);
    generate_excellon(board, output_dir);
    
    printf("Drill file generation complete\n");
    return 0;
}

// Main program entry point
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
        
        const char* output_dir = argc > 2 ? argv[2] : "./output";
        
        generate_gerber_files(current_board, output_dir);
        generate_drill_files(current_board, output_dir);
        
        free_board(current_board);
        
        return EXIT_SUCCESS;
    } else {
        printf("Parsing failed.\n");
        return EXIT_FAILURE;
    }
}