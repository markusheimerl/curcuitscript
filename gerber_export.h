#ifndef GERBER_EXPORT_H
#define GERBER_EXPORT_H

#include "pcb_model.h"

int generate_gerber_files(Board* board, const char* output_dir);
int generate_drill_files(Board* board, const char* output_dir);

#endif