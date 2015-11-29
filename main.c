#include "print.h"
#include "issue.h"
#include "exec.h"
#include "writeback.h"
#include "commit.h"
#include "parser.h"
#include "timingtable.h"
#include "lsq.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct timetable_line *TimeTable;
extern int table_index;

struct RS_ RS; // register station
struct LsQueue LSQ; // Load/Store Queue
struct ALU_ ALU;
struct ROB_ ROB;
struct RAT_line *RAT;
struct input_instr *instr_mem;
float *data_mem;
float *RF;
int *int_RF;
float *float_RF;


int main(int argc, char **argv) {

    // 1.test arguments is valid or not
    //
    if (argc != 3) {
        printf("Usage: %s <input_file> <global configuration file>\n", argv[0]);
        exit(1);
    }
    // 1.1 Initialize all parameters and allocate memory, (some memory are allocated in Parse_File())
    char *input_file_name = argv[1];
    char *conf_file_name = argv[2];

    //Memory allocation
    TimeTable = (struct timetable_line *) malloc (MAX_TTABLE_LINE * sizeof(struct timetable_line));
    memset(TimeTable, 0, MAX_TTABLE_LINE * sizeof(struct timetable_line));
    instr_mem = (struct input_instr *) malloc(MEM_SIZE * sizeof(struct input_instr));
    memset(instr_mem, 0, MEM_SIZE * sizeof(struct input_instr));
    data_mem = (float *) malloc(MEM_SIZE * sizeof(float));
    memset(data_mem, 0, MEM_SIZE * sizeof(float));

    //RF allocation
    float *RF = (float *) malloc(2 * ARF_SIZE * sizeof(int));
    memset(RF, 0, 2 * ARF_SIZE * sizeof(int));
    int_RF = (int *) RF;
    float_RF = (float *) (RF + ARF_SIZE);
    RAT = (struct RAT_line *) malloc(2 * ARF_SIZE * sizeof(struct RAT_line));
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
    if (Parse_File(input_file_name, conf_file_name)) {
         printf("Read file failed.\n");
         exit(1);
    }
    //DEBUG: print status
    if (printStatus(cycles)) {
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

        // if ROB is busy, continue
        for (int i =0; i < ROB.size; ++i) {
            if (notFinishROB(&ROB.entity[i])) {
                done = FALSE;
                break;
            }
        }

        //start commit
        //Requirement:
        //next commit header to ROB's is finished
        if (readyCommitROB(ROB.entity[ROB.nextcommit])) {
            done = FALSE;
            startCommit(&ROB.entity[ROB.nextcommit], cycles);
            ROB.nextcommit = (ROB.nextcommit + 1 + ROB.size) % ROB.size;
        }

        //start writeback
        //Requirement:
        //1.exec is complete
        //2.cdb is free (?)
        for (int i = 0; i < RS.size; ++i) {
            struct RS_line *this_RS = &RS.entity[i];
            if (exeCompleteALU(this_RS, cycles) && cdb_free) {
                done = FALSE;
                cdb_free = FALSE;
                // actually, the result is write back in this cycles
                startWback(this_RS, cycles);
            }
        }
        cdb_free = TRUE;

        // 4.2 start exec stage
        // Requirement:
        // 1.issue exe 1 cycle
        // 2.have room in ALU
        // 3.all data are ready
        for (int i = 0; i < RS.size; ++i) {
            struct RS_line *this_RS = &RS.entity[i];
            startExecALU(this_RS, cycles);
        }

        // 4.1 ISSUE to RS
        if (has_instr(PC)) {
            done = FALSE;
            struct input_instr this_instr = instr_mem[PC];
            // ALU instructions
            if (isALUIns(this_instr)) {
                if (issueALU(this_instr, cycles)) {
                    ROB.nextfree = (ROB.nextfree + 1 + ROB.size) % ROB.size;
                    PC++;
                }
            } else if(isLSIns(this_instr)) {
                if (issueLS(this_instr, cycles)) {
                    PC++;
                }
            } else {
                printf("unknown instruction: %d\n" ,this_instr.op);
                exit(1);
            }

        }

        //print current status
        printStatus(cycles);
        printTimetable();
        printf("Press ENTER to continue.\n");
        getchar();

    }
    printStatus(cycles);

    // print Timing Table
    printTimetable();

    return 0;
}

