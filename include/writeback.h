#ifndef WRITEBACK_H_
#define WRITEBACK_H_
#include "util.h"

// whether this_RS is exec complete or not
int exeCompleteALU(struct RS_line *this_RS, int cycles);

// Turn this exec complete instr to wback
int startWback(struct RS_line *this_RS,
        int cycles);

// get result from ALU
float getResultALU(struct RS_line *this_RS);
#endif
