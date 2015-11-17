#ifndef UPPER_H_
#define UPPER_H_
#include "util.h"
#include "instr.h"
#include "lower.h"
#include <stdio.h>

/*
 * function @ parse_file read files and initialize all parameters
 * input: FILE *fp
 * output: instr_array
 * output: ARF | Register Files
 * output: mem | a bunch of memory
 * TODO
 */

int Parse_File(char *file_name,
               struct input_instr *instr_mem,
               float *data_mem,
               struct RS_line *RS,
               int *RS_size,
               float *float_RF,
               int *int_RF,
               struct ROB_line *ROB,
               int *ROB_size,
               struct ALU_line *ALU,
               int *ALU_size);
#endif
