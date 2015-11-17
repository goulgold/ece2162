#include <stdio.h>
#include "upper.h"
#include <stdlib.h>

int main(int argc, char **argv) {

    // 1.test arguments is valid or not
    //
    if (argc != 2) {
        printf("Usage: %s <input_file>\n", argv[0]);
        exit(1);
    }
    // 1.1 Initialize all parameters and allocate memory, (some memory are allocated in Parse_File())
    char *file_name = argv[1];
    struct RS_line *RS; // register station
    int RS_size;
    struct ALU_line *ALU;
    int ALU_size;
    struct input_instr *instr_mem = (struct input_instr *) malloc(MEM_SIZE * sizeof(struct input_instr));
    memset(instr_mem, 0, MEM_SIZE * sizeof(struct input_instr));
    float *data_mem = (float *) malloc(MEM_SIZE * sizeof(float));
    memset(data_mem, 0, MEM_SIZE * sizeof(float));
    float *float_RF = (float *) malloc(ARF_SIZE * sizeof(float));
    memset(float_RF, 0, ARF_SIZE * sizeof(float));
    int *int_RF = (int *) malloc(ARF_SIZE * sizeof(int));
    memset(int_RF, 0, ARF_SIZE * sizeof(int));
    struct RAT_line *RAT = (struct RAT_line *) malloc(2 * ARF_SIZE * sizeof(struct RAT_line));
    memset(RAT, 0, 2 * ARF_SIZE * sizeof(struct RAT_line));
    struct ROB_line *ROB;
    int ROB_size;
    int cycles = 0;
    int PC = 0;
    // next commit ROB
    int ROB_nextcommit = 0;
    // next available ROB
    int ROB_nextfree = 0;
    // CDB
    int cdb_free = 1;

    // 2.parse arguments load instr to memory
    // 3.initialize： RS, ROB, ARF, Timing Table, Memory
    if (Parse_File(file_name, instr_mem, data_mem, &RS, &RS_size, float_RF, int_RF, &ROB, &ROB_size, &ALU, &ALU_size)) {
         printf("Read file failed.\n");
         exit(1);
    }
    //DEBUG: print status
    if (printStatus(instr_mem, data_mem, RS, RS_size, float_RF, int_RF, ROB, ROB_size, ALU, ALU_size, RAT)) {
        printf("Print Status failed.\n");
        exit(1);
    }

    // 4.Start simulate until PC point to NULL
    while (has_instr(instr_mem, PC) || !ROB_empty(ROB, ROB_size)) {

        // 4.1 ISSUE to RS
        instr2RS(instr_mem, &PC, RS, RS_size, ROB, ROB_size, RAT, &ROB_nextfree);

        // 4.2 issue stage to exec stage
        // Requirement:
        // 1.issue exe 1 cycle
        // 2.have room in ALU
        // 3.all data are ready
        toExec(RS, RS_size, ALU, ALU_size, ROB, ROB_size);

        //exec stage to writeback
        //Requirement:
        //1.exec is complete
        //2.cdb is free (?)
        toWback(RS, RS_size, ALU, ALU_size, ROB, ROB_size, cdb_free);

        //writeback stage to commit
        toCommit(ROB, &ROB_nextcommit, RAT, int_RF, float_RF);

        cycles++;
    }
/*  **
    while (has_next(instr_array) || not_empty(ROB)) {

        get next_instr;

        // insert this instr into RS and update RAT & ROB
        if (has_seat(RS, next_instr) && has_seat(ROB, next_instr)) {

            // use RAT to update instr
            if (operandof(next_instr) is in ARF) {
                load value directly.
            } else {
                 load ROB location.
            }
            // insert this instr into RS
            insert(next_instr, RS);
            // update ROB & RAT
            update dest of next_instr in RAT.
            update dest of next_instr in ROB.
        }

        //update all buffer in this function
        add_cycle(RS)
            if (any_instr in RS ready to exec) {
                start_exec(this_instr);
            }
            if (any_instr in RS ready to write back && CDB is free) {
                start_writeback(this_instr);
            }
            if (write back in CDB is complete) {
                update ROB;
                remove this instr from RS;
                update RS waiting for this result;
            }

    }
    **/
    return 0;
}

