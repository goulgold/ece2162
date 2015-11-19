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
void getIRF(char *line, int *int_RF);
void getFRF(char *line, float *float_RF);

//given a string, update Memory
void getMem(char *line, float *data_mem);

//given a file pointer, update instr_mem
void getInstr(FILE *fp, struct input_instr *instr_mem);

//given a string, update ONE instr_line
void getInstrline(char *line, struct input_instr *instr_mem, int index);

// is a Float instruction, return true
int isFloatInstr(int op);

// add a instruction into RS and update ROB RAT
int addRSROB(int alu_type,
             struct input_instr thisInstr,
             struct RS_line *RS,
             int RS_size,
             struct ROB_line *ROB,
             int ROB_size,
             int *ROB_nextfree,
             struct RAT_line *RAT,
             int PC,
             float *RF);
