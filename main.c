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
    struct RS_ RS; // register station
    struct ALU_ ALU;
    struct ROB_ ROB;

    //Memory allocation
    struct input_instr *instr_mem = (struct input_instr *) malloc(MEM_SIZE * sizeof(struct input_instr));
    memset(instr_mem, 0, MEM_SIZE * sizeof(struct input_instr));
    float *data_mem = (float *) malloc(MEM_SIZE * sizeof(float));
    memset(data_mem, 0, MEM_SIZE * sizeof(float));

    //RF allocation
    float *RF = (float *) malloc(2 * ARF_SIZE * sizeof(int));
    memset(RF, 0, 2 * ARF_SIZE * sizeof(int));
    int *int_RF = (int *) RF;
    float *float_RF = (float *) (RF + ARF_SIZE);
    struct RAT_line *RAT = (struct RAT_line *) malloc(2 * ARF_SIZE * sizeof(struct RAT_line));
    memset(RAT, 0, 2 * ARF_SIZE * sizeof(struct RAT_line));

    int cycles = 0;
    int PC = 0;
    // CDB
    int cdb_free = 1;
    // whether need next cycle or not
    int done = FALSE;

    // 2.parse arguments load instr to memory
    // 3.initialize： RS, ROB, ARF, Timing Table, Memory
    // next available ROB
    if (Parse_File(file_name, instr_mem, data_mem, &RS, int_RF, float_RF, &ROB, &ALU)) {
         printf("Read file failed.\n");
         exit(1);
    }
    //DEBUG: print status
    if (printStatus(instr_mem, data_mem, &RS, float_RF, int_RF, &ROB, &ALU, RAT,cycles)) {
        printf("Print Status failed.\n");
        exit(1);
    }

    // 4.Start simulate until no more cycle is needed.
    //
    //
    //
    while (!done) {
        cycles++;
        done = TRUE;
        // 4.1 ISSUE to RS
        if (has_instr(instr_mem, PC)) {
            done = FALSE;
            struct input_instr this_instr = instr_mem[PC];
            // ALU instructions
            if (isALUIns(this_instr)) {
                if (issueALU(this_instr, RS, ROB, RAT, int_RF, float_RF, cycles)) {
                    ROB.nextfree = (ROB.nextfree + 1 + ROB.size) % ROB.size;
                    PC++;
                }
            } else {
                printf("unknown instruction: %d\n" ,this_instr.op);
                exit(1);
            }

        }

        // 4.2 start exec stage
        // Requirement:
        // 1.issue exe 1 cycle
        // 2.have room in ALU
        // 3.all data are ready
        for (int i = 0; i < RS.size; ++i) {
            struct RS_line *this_RS = &RS.entity[i];
            startExecALU(this_RS, ALU, cycles);
        }

        //start commit
        //Requirement:
        //next commit header to ROB's is finished
        if (readyCommitROB(ROB.entity[ROB.nextcommit])) {
            done = FALSE;
            startCommit(&ROB.entity[ROB.nextcommit], RAT, int_RF, float_RF);
            ROB.nextcommit = (ROB.nextcommit + 1 + ROB.size) % ROB.size;
        }

        //start writeback
        //Requirement:
        //1.exec is complete
        //2.cdb is free (?)
        for (int i = 0; i < RS.size; ++i) {
            struct RS_line *this_RS = &RS.entity[i];
            if (exeCompleteALU(this_RS, ALU, cycles) && cdb_free) {
                done = FALSE;
                cdb_free = FALSE;
                // actually, the result is write back in this cycles
                startWback(this_RS, RS, ALU, ROB, cycles);
            }
        }
        cdb_free = TRUE;

        //print current status
        printStatus(instr_mem, data_mem, &RS, float_RF, int_RF, &ROB, &ALU, RAT,cycles);
        printf("Press ENTER to continue.\n");
        getchar();
    }


    return 0;
}

