#ifndef PARSER_H_
#define PARSER_H_
#include "util.h"

/*
 * function @ parse_file read files and initialize all parameters
 * input: FILE *fp
 * output: instr_array
 * output: ARF | Register Files
 * output: mem | a bunch of memory
 * TODO
 */

int Parse_File(const FILE *fp,
               struct instr *instr_array,
               int *instr_array_size,
               float *mem,
               const int *mem_size,
               float *ARF,
               const int *ARF_size,
               int *ROB_size);

#endif
