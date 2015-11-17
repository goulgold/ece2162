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
               struct RS_line **RS,
               int *RS_size,
               float *float_RF,
               int *int_RF,
               struct ROB_line **ROB,
               int *ROB_size,
               struct ALU_line **ALU,
               int *ALU_size);

/*
 * function @printStatus print all information
 * TODO
 */

int printStatus(struct input_instr *instr_mem,
                float *data_mem,
                struct RS_line *RS,
                int RS_size,
                float *float_RF,
                int *int_RF,
                struct ROB_line *ROB,
                int ROB_size,
                struct ALU_line *ALU,
                int ALU_size,
                struct RAT_line *RAT);

//PC pointer's location, has a instr or not
int has_instr(struct input_instr *instr_array, int PC);

//ROB is empty or not
int ROB_empty(struct ROB_line *ROB, int ROB_size);

// issue instr into RS, update RS, ROB, RAT
int instr2RS(struct input_instr *instr_mem,
             int *PC,
             struct RS_line *RS,
             int RS_size,
             struct ROB_line *ROB,
             int ROB_size,
             struct RAT_line *RAT,
             int *ROB_nextfree);

// Turn all issue instr to exec as many as possible
// Before write back
int toExec(struct RS_line *RS,
           int RS_size,
           struct ALU_line *ALU,
           int ALU_size,
           struct ROB_line *ROB,
           int ROB_size);
// Turn all exec complete instr to wback
int toWback(struct RS_line *RS,
        int RS_size,
        struct ALU_line *ALU,
        int ALU_size,
        struct ROB_line *ROB,
        int ROB_size,
        int cdb_free);


int toCommit(struct ROB_line *ROB,
             int *ROB_nextcommit,
             struct RAT_line *RAT,
             int *int_RF,
             float *float_RF);
#endif
