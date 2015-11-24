#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "instr.h"

// given a string, read offset location integer and return
int getNum(char *string, int offset);
float getNumf(char *line, int offset);

//give a string, obtain alu information
int getALUtype(char *line);
int getRsnum(char *line);
int getExcyc(char *line);
int getMemcyc(char *line);
int getFunum(char *line);

//given a string, update Integer RF and Float RF
int getIRF(char *line, int *int_RF);
int getFRF(char *line, float *float_RF);

//given a string, update Memory
int getMem(char *line, float *data_mem);

//given a file pointer, update instr_mem
void getInstr(FILE *fp, struct input_instr *instr_mem);

//given a string, update ONE instr_line
void getInstrline(char *line, struct input_instr *instr_mem, int index);

// print a ROB pointer
void printPointROB(struct ROB_line *this_ROB);

// is a Float instruction, return true
int isFloatInstr(int op);

// given a instruction, get ALU type
int instr2ALUtype(struct input_instr this_instr);

// ROB has seat or not
int hasSeatRS(struct input_instr this_instr, struct RS_ RS);

int hasSeatROB(struct ROB_ ROB);

// ALU has a seat or not
int hasSeatALU(struct RS_line *this_RS, struct ALU_ ALU);

//Whether all data in this_RS is ready
int dataReadyRS(struct RS_line *this_RS);

//ISSUE stage is complete (1 cycle has passed.)
int issueComplete(struct RS_line *this_RS, int cycles);

// get result from ALU
float getResultALU(struct RS_line *this_RS);
