#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gerber_export.h"

// Make sure the output directory exists
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

// Helper function to write Gerber header
static void write_gerber_header(FILE* fp, const char* type) {
    // Format specification
    fprintf(fp, "%%FSLAX46Y46*%%\n");  // Leading zeros omitted, absolute coords, 4.6 format
    
    // Unit setting
    fprintf(fp, "%%MOMM*%%\n");  // Units = millimeters
    
    // File function
    fprintf(fp, "%%TF.FileFunction,%s*%%\n", type);
    
    // Part
    fprintf(fp, "%%TF.Part,Single*%%\n");
    
    // Creator information
    fprintf(fp, "%%TF.CreationDate,20230101T120000*%%\n");
    fprintf(fp, "%%TF.GenerationSoftware,CircuitScript,1.0*%%\n");
    
    // Clear aperture list
    fprintf(fp, "%%MOIN*%%\n");  // Set units to inches (common for aperture defs)
    
    // Define apertures (simplified for this example)
    fprintf(fp, "%%ADD10C,0.010*%%\n");  // Aperture 10: Circle with 0.01" diameter
    fprintf(fp, "%%MOMM*%%\n");  // Switch back to mm
}

// Helper function to write Gerber footer
static void write_gerber_footer(FILE* fp) {
    fprintf(fp, "M02*\n");  // End of file
}

// Generate outline Gerber (board shape)
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
    
    // Draw board outline (simple rectangle)
    fprintf(fp, "G01*\n");  // Linear interpolation mode
    fprintf(fp, "D10*\n");  // Use aperture 10
    
    int x1 = 0;
    int y1 = 0;
    int x2 = (int)(board->width * 1000000);  // Convert mm to 0.1 µm for Gerber
    int y2 = (int)(board->height * 1000000);
    
    // Move to first corner
    fprintf(fp, "X%dY%dD02*\n", x1, y1);
    
    // Draw the rectangle
    fprintf(fp, "X%dY%dD01*\n", x2, y1);  // Draw to bottom right
    fprintf(fp, "X%dY%dD01*\n", x2, y2);  // Draw to top right
    fprintf(fp, "X%dY%dD01*\n", x1, y2);  // Draw to top left
    fprintf(fp, "X%dY%dD01*\n", x1, y1);  // Draw back to starting point
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated outline Gerber: %s\n", filename);
    return 0;
}

// Generate copper layer Gerber
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
    
    // This is where you would draw pads for components and traces
    // For this simplified example, we'll just place pads for components
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        // Skip components on the wrong side
        if (placement->top_side != top_layer) continue;
        
        // Find the component definition
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
        
        // Place a pad for each pin (simplified - in reality you'd use the component's footprint)
        for (int j = 0; j < component->pin_count; j++) {
            Pin* pin = &component->pins[j];
            
            // Calculate pin position (simplified)
            double pin_x = placement->position.x + j * 2.54;  // Space pins at 2.54mm (100mil)
            double pin_y = placement->position.y;
            
            // Convert to Gerber coordinates (0.1 µm)
            int x = (int)(pin_x * 1000000);
            int y = (int)(pin_y * 1000000);
            
            // Draw the pad (flash aperture)
            fprintf(fp, "X%dY%dD03*\n", x, y);
        }
    }
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated copper layer Gerber: %s\n", filename);
    return 0;
}

// Generate silkscreen layer Gerber
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
    
    // In a real implementation, you would draw component outlines and labels here
    // For this example, we'll just draw a simple outline around each component
    
    fprintf(fp, "G01*\n");  // Linear interpolation mode
    fprintf(fp, "D10*\n");  // Use aperture 10
    
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        // Skip components on the wrong side
        if (placement->top_side != top_layer) continue;
        
        // Draw a simple rectangle around the component (10mm x 5mm)
        int x1 = (int)((placement->position.x - 5) * 1000000);
        int y1 = (int)((placement->position.y - 2.5) * 1000000);
        int x2 = (int)((placement->position.x + 5) * 1000000);
        int y2 = (int)((placement->position.y + 2.5) * 1000000);
        
        // Move to first corner
        fprintf(fp, "X%dY%dD02*\n", x1, y1);
        
        // Draw the rectangle
        fprintf(fp, "X%dY%dD01*\n", x2, y1);
        fprintf(fp, "X%dY%dD01*\n", x2, y2);
        fprintf(fp, "X%dY%dD01*\n", x1, y2);
        fprintf(fp, "X%dY%dD01*\n", x1, y1);
        
        // Draw the reference designator text
        // (In a real implementation, this would use specialized text functions)
        int text_x = (int)(placement->position.x * 1000000);
        int text_y = (int)(placement->position.y * 1000000);
        fprintf(fp, "X%dY%dD02*\n", text_x, text_y);
    }
    
    write_gerber_footer(fp);
    fclose(fp);
    
    printf("Generated silkscreen Gerber: %s\n", filename);
    return 0;
}

// Generate drill file (Excellon format)
static int generate_excellon(Board* board, const char* output_dir) {
    char filename[256];
    snprintf(filename, sizeof(filename), "%s/%s.drl", 
             output_dir, board->name ? board->name : "board");
    
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create Excellon drill file: %s\n", filename);
        return -1;
    }
    
    // Write Excellon header
    fprintf(fp, "M48\n");  // Program header
    fprintf(fp, "METRIC,TZ\n");  // Use metric units, trailing zeros
    fprintf(fp, "T1C1.000\n");  // Tool 1 has a 1.00mm diameter
    fprintf(fp, "%\n");  // End of header
    fprintf(fp, "G90\n");  // Use absolute coordinates
    fprintf(fp, "G05\n");  // Drill mode
    fprintf(fp, "T1\n");  // Select tool 1
    
    // For each component, generate drill holes for each pin
    for (int i = 0; i < board->placement_count; i++) {
        Placement* placement = &board->placements[i];
        
        // Find the component definition
        Component* component = NULL;
        for (int j = 0; j < board->component_count; j++) {
            if (strcmp(board->components[j].name, placement->component_name) == 0) {
                component = &board->components[j];
                break;
            }
        }
        
        if (!component) continue;
        
        // Place a drill hole for each pin (simplified)
        for (int j = 0; j < component->pin_count; j++) {
            // Calculate pin position (simplified)
            double pin_x = placement->position.x + j * 2.54;  // Space pins at 2.54mm
            double pin_y = placement->position.y;
            
            // Write drill command
            fprintf(fp, "X%.3fY%.3f\n", pin_x, pin_y);
        }
    }
    
    // Write Excellon footer
    fprintf(fp, "T0\n");  // Cancel tool
    fprintf(fp, "M30\n");  // End of program
    
    fclose(fp);
    
    printf("Generated Excellon drill file: %s\n", filename);
    return 0;
}

// Main function to generate all Gerber files
int generate_gerber_files(Board* board, const char* output_dir) {
    if (!board) {
        fprintf(stderr, "Error: No board data to generate Gerber files\n");
        return -1;
    }
    
    ensure_directory_exists(output_dir);
    
    // Generate different layers
    generate_outline(board, output_dir);
    generate_copper_layer(board, output_dir, true);  // Top copper
    
    if (board->layers >= 2) {
        generate_copper_layer(board, output_dir, false);  // Bottom copper
    }
    
    generate_silkscreen(board, output_dir, true);  // Top silkscreen
    
    if (board->layers >= 2) {
        generate_silkscreen(board, output_dir, false);  // Bottom silkscreen
    }
    
    printf("Gerber files generation complete\n");
    return 0;
}

// Main function to generate drill files
int generate_drill_files(Board* board, const char* output_dir) {
    if (!board) {
        fprintf(stderr, "Error: No board data to generate drill files\n");
        return -1;
    }
    
    ensure_directory_exists(output_dir);
    
    // Generate the drill file
    generate_excellon(board, output_dir);
    
    printf("Drill file generation complete\n");
    return 0;
}