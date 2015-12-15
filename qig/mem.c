#include "mem.h"
#include "instr.h"
#include "timingtable.h"

extern int table_index;

extern struct input_instr *instr_mem;
extern struct RS_ RS;
extern struct LsQueue LSQ;
extern struct ROB_ ROB;
extern struct RAT_line *RAT;
extern int *int_RF;
extern float *float_RF;
extern float *data_mem;
extern struct timetable_line *TimeTable;

int dataReadyLSQ(struct LsQueue_line *this_LSQ) {
    if (this_LSQ->instr_type == LD) return TRUE;
    if (this_LSQ->instr_type == SD && this_LSQ->data_ready == TRUE) return TRUE;
    else return FALSE;
}

int startMemLSQ(struct LsQueue_line *this_LSQ, int cycles, int *membus_free) {
    // Load TODO forwarding value from store
    if (this_LSQ->instr_type == LD && this_LSQ->stage == EXEC) {
        this_LSQ->stage = MEM;
        this_LSQ->cycle = cycles;
        this_LSQ->mem_val = data_mem[this_LSQ->mem_addr];
        *membus_free = FALSE;
        // update TimingTable
        TimeTable[this_LSQ->ttable_index].mem = cycles;
    }
    // Store don't have MEM stage, hop to ready to COMMIT status
    if (this_LSQ->instr_type == SD && this_LSQ->stage == EXEC) {
       //TODO
        this_LSQ->stage = MEM;
        this_LSQ->cycle = cycles;
        // update TimingTable
        TimeTable[this_LSQ->ttable_index].mem = cycles;
    }
    return TRUE;
}
