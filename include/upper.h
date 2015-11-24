#ifndef UPPER_H_
#define UPPER_H_
#include "util.h"
#include "instr.h"
#include "lower.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * function @ parse_file read files and initialize all parameters
 * input: FILE *fp
 * output: instr_array
 * output: ARF | Register Files
 * output: mem | a bunch of memory
 * TODO
 */

int Parse_File(char *input_file_name,
               char *conf_file_name,
               struct input_instr *instr_mem,
               float *data_mem,
               struct RS_ *RS,
               int *int_RF,
               float *float_RF,
               struct ROB_ *ROB,
               struct ALU_ *ALU);

/*
 * function @printStatus print all information
 * TODO
 */

int printStatus(struct input_instr *instr_mem,
                float *data_mem,
                struct RS_ *RS,
                float *float_RF,
                int *int_RF,
                struct ROB_ *ROB,
                struct ALU_ *ALU,
                struct RAT_line *RAT,
                int cycles);

//PC pointer's location, has a instr or not
int has_instr(struct input_instr *instr_array, int PC);

//ROB is empty or not
int ROB_empty(struct ROB_line *ROB, int ROB_size);

// this instruction is normal or not.
int isALUIns(struct input_instr instr_mem);

// issue instr into RS, update RS, ROB, RAT, ALU
int issueALU(struct input_instr this_instr,
             struct RS_ RS,
             struct ROB_ ROB,
             struct RAT_line *RAT,
             int *int_RF,
             float *float_RF,
             int cycles);



// Turn all issue instr to exec as many as possible
// Before write back
int startExecALU(struct RS_line *this_RS,
                 struct ALU_ ALU,
                 int cycles);

// whether this_RS is exec complete or not
int exeCompleteALU(struct RS_line *this_RS, struct ALU_ ALU, int cycles);

// Turn this exec complete instr to wback
int startWback(struct RS_line *this_RS,
        struct RS_ RS,
        struct ALU_ ALU,
        struct ROB_ ROB,
        int cycles);

int readyCommitROB(struct ROB_line this_ROB);

int startCommit(struct ROB_line *this_ROB,
             struct RAT_line *RAT,
             int *int_RF,
             float *float_RF,
             int cycles);

int notFinishROB(struct ROB_line *this_ROB);
#endif
