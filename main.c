#include "print.h"
#include "issue.h"
#include "exec.h"
#include "mem.h"
#include "writeback.h"
#include "commit.h"
#include "parser.h"
#include "timingtable.h"
#include "lsq.h"
#include "instr.h"
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
struct pipe_state pipe;
int PC;
buffer_t btb[BTB_SIZE];
int stall_commit; // commit stage is stall or not



int main(int argc, char **argv) {

    // 1.test arguments is valid or not
    //
    if (argc != 3) {
        printf("Usage: %s <input_file> <global configuration file>\n", argv[0]);
        argv[1] = "testcase/b1.txt";
        argv[2] = "testcase/global.conf";
        //exit(1);
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

    for (int i=0; i<BTB_SIZE; i++){
        btb[i].valid=0;
        btb[i].tag=0;
        btb[i].target=0;
    }

    int cycles = 0;
    PC = 0;
    pipe.branch_index = -1;
    stall_commit = FALSE;
    // CDB
    int cdb_free = TRUE;
    int membus_free = TRUE;
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

        //handle pipeline recover
        if(pipe.branch_recover){
            /* clear ROB entries */
            memset(ROB.entity, 0, ROB.size * sizeof(struct ROB_line));
            for (int i = 0; i < ROB.size; ++i) {
                ROB.entity[i].index = i+1;
            }
            ROB.nextfree = 0;
            ROB.nextcommit = 0;

            /* reset RAT*/
            memset(RAT, 0, 2 * ARF_SIZE * sizeof(struct RAT_line));

            /* reset RS*/
            for (int i = 0; i < RS.size; ++i) {
                resetRS(&RS.entity[i]);
            }

            //update PC
            PC = pipe.branch_dest;
            //update pipeline status
            pipe.branch_recover = 0;
            pipe.branch_dest = 0;
            pipe.branch_index = -1;
            pipe.branch_unresovled = 0;
            done = FALSE;
            // reset ALU
            for (int i = 0; i < ALU.size; ++i) {
                ALU.entity[i].busy = FALSE;
            }
            // reset LSQ
            resetLSQ(LSQ);
            continue;
        }




        //start commit
        //Requirement:
        //next commit header to ROB's is finished
        if (!stall_commit && readyCommitROB(ROB.entity[ROB.nextcommit])) {
            done = FALSE;
            startCommit(&ROB.entity[ROB.nextcommit], cycles);
            if (ROB.nextcommit == pipe.branch_index) stall_commit = TRUE;
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

        // start writeback Load / Store instruction
        for (int i = 0; i < LSQ.size; ++i) {
            struct LsQueue_line *this_LSQ = &LSQ.entity[i];
            // Load instruction
            if (this_LSQ->instr_type == LD && memCompleteLoad(this_LSQ, cycles) && cdb_free && this_LSQ->stage == MEM) {
                done = FALSE;
                cdb_free = FALSE;
                startWbackLoad(this_LSQ, cycles);
                membus_free = TRUE;
            }
            if (this_LSQ->instr_type == SD && this_LSQ->data_ready == TRUE && this_LSQ->stage == MEM) {
            // TODO Store instruction
                done = FALSE;
                this_LSQ->buffer->val = this_LSQ->mem_val;
                this_LSQ->buffer->finished = TRUE;
                this_LSQ->stage = WBACK;
                this_LSQ->cycle = cycles;
            }
        }
        cdb_free = TRUE;

        // Memory stage
        for (int i = 0; i < LSQ.size; ++i) {
            if (membus_free == TRUE && dataReadyLSQ(&LSQ.entity[i])) {
                startMemLSQ(&LSQ.entity[i], cycles, &membus_free);
            }
        }

        // 4.2 start exec stage
        // Requirement:
        // 1.issue exe 1 cycle
        // 2.have room in ALU
        // 3.all data are ready
        for (int i = 0; i < RS.size; ++i) {
            struct RS_line *this_RS = &RS.entity[i];
            startExecALU(this_RS, cycles);
        }

        // 4.3 start exec load / store instruction
        for (int i = 0; i < LSQ.size; ++i) {
            startExecLS(&LSQ.entity[i], cycles);
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
            } else if (isBranchIns(this_instr)) {
                issueBranch(this_instr, cycles);
            } else if (isLSIns(this_instr)) {
                if (issueLS(this_instr, cycles)) {
                    ROB.nextfree = (ROB.nextfree + 1 + ROB.size) % ROB.size;
                    LSQ.head = (LSQ.head + 1) % LSQ.size;
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

