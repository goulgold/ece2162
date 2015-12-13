#include "exec.h"
#include "timingtable.h"
#include "instr.h"
#include <stdio.h>
#include <stdlib.h>

extern struct ALU_ ALU;
extern struct LsQueue LSQ;
extern struct timetable_line *TimeTable;


int startExecALU(struct RS_line *this_RS,
                 int cycles) {
    //TODO
    int seatALU = hasSeatALU(this_RS);

    if (this_RS->busy == TRUE &&
        dataReadyRS(this_RS) &&
        issueComplete(this_RS, cycles) &&
        seatALU != -1 &&
        this_RS->stage == ISSUE) {

        //update timing table
        startEXECtable(this_RS->ttable_index, cycles);

        //update RS
        this_RS->stage = EXEC;
        this_RS->cycles = cycles;
        this_RS->alu_index = seatALU;

        //update ALU
        ALU.entity[seatALU].busy = TRUE;

        return TRUE;
    }

    return FALSE;
}

int hasSeatALU(struct RS_line *this_RS) {
    for (int i = 0; i < ALU.size; ++i) {
        if (ALU.entity[i].type == this_RS->alu_type && ALU.entity[i].busy == FALSE)
        return i;
    }
    return -1;
}

int dataReadyRS(struct RS_line *this_RS) {
    if (this_RS->tag_1 == NULL && this_RS->tag_2 == NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int issueComplete(struct RS_line *this_RS, int cycles) {
    if (cycles - this_RS->cycles > 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int startExecLS(struct LsQueue_line *this_LSQ, int cycles) {

    if (this_LSQ->busy == TRUE &&
        baseAddrReadyLSQ(this_LSQ) &&
        this_LSQ->stage == ISSUE) {
        this_LSQ->mem_addr = this_LSQ->val_1 + this_LSQ->offset;
        this_LSQ->stage = EXEC;
        this_LSQ->cycle = cycles;
        // update TimingTable
        TimeTable[this_LSQ->ttable_index].exec = cycles;
        // update ROB of store instr
        if (this_LSQ->instr_type == SD) {
            this_LSQ->buffer->addr = this_LSQ->mem_addr;
        }
        return TRUE;
    } else
        return FALSE;
}

int baseAddrReadyLSQ(struct LsQueue_line *this_LSQ) {
    return (this_LSQ->tag_1 == NULL);
}
