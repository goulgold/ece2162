#include "exec.h"
#include "timingtable.h"
#include "instr.h"
#include <stdio.h>
#include <stdlib.h>

extern struct ALU_ ALU;

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
