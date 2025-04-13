#ifndef GERBER_EXPORT_H
#define GERBER_EXPORT_H

#include "pcb_model.h"

// Function to generate Gerber files from board model
int generate_gerber_files(Board* board, const char* output_dir);

// Function to generate Excellon drill file from board model
int generate_drill_files(Board* board, const char* output_dir);

#endif // GERBER_EXPORT_H