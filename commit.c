#include "commit.h"
#include "timingtable.h"

extern struct RAT_line *RAT;
extern int *int_RF;
extern float *float_RF;


int readyCommitROB(struct ROB_line this_ROB) {
    if (this_ROB.finished == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int startCommit(struct ROB_line *this_ROB,
             int cycles) {
     //TODO
    // update timing table
    startCOMMITtable(this_ROB->ttable_index, cycles);
    //update ROB
    this_ROB->busy = FALSE;
    this_ROB->finished = FALSE;
    // update RAT
    RAT[this_ROB->dst].tag = 0;
     // int RF
    if (this_ROB->dst < ARF_SIZE) {
        int result = (int) this_ROB->val;
        // update int_RF
        int_RF[this_ROB->dst] = result;
    } else if (this_ROB->dst >= ARF_SIZE) {
        float result = (float) this_ROB->val;
        // update float_RF
        float_RF[this_ROB->dst - ARF_SIZE] = result;
    }
    return TRUE;
}
