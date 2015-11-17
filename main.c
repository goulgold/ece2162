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
    // 1.1 Initialize all parameters and allocate memory
    char *file_name = argv[1];
    struct RS_line *RS; // register station
    int RS_size;
    struct ALU_line *ALU;
    int ALU_size;
    struct input_instr *instr_mem = (struct input_instr *) malloc(MEM_SIZE * sizeof(struct input_instr));
    float *data_mem = (float *) malloc(MEM_SIZE * sizeof(float));
    float *float_RF = (float *) malloc(ARF_SIZE * sizeof(float));
    int *int_RF = (int *) malloc(ARF_SIZE * sizeof(int));
    struct RAT_line *RAT = (struct RAT_line *) malloc(2 * ARF_SIZE * sizeof(struct RAT_line));
    struct ROB_line *ROB;
    int ROB_size;
    int cycles = 0;
    if (Parse_File(file_name, instr_mem, data_mem, RS, &RS_size, float_RF, int_RF, ROB, &ROB_size, ALU, &ALU_size)) {
         printf("Read file failed.\n");
         exit(1);
    }

    // 2.parse arguments load instr to memory
    //
    // 3.initialize： RS, ROB, ARF, Timing Table, Memory
    //
    // 4.Start simulate until PC point to NULL
    // 4.1 ISSUE
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

