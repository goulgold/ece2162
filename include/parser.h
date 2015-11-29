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

int Parse_File(char *input_file_name,
               char *conf_file_name);

// given a string, read offset location integer and return
int getNum(char *string, int offset);
float getNumf(char *line, int offset);

//give a string, obtain alu information
int getALUtype(char *line);
int isALUunit(int alu_type);
int getRsnum(char *line);
int getExcyc(char *line);
int getMemcyc(char *line);
int getFunum(char *line);

//given a string, update Integer RF and Float RF
int getRF(char *line, int *int_RF, float *float_RF);
int getIRF(char *line, int *int_RF);
int getFRF(char *line, float *float_RF);

//given a string, update Memory
int getMem(char *line, float *data_mem);

//given a string, update ONE instr_line
int getInstrline(char *line, struct input_instr *instr_mem, int index);

#endif
