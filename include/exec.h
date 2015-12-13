#ifndef EXEC_H_
#define EXEC_H_
#include "util.h"

// Turn all issue instr to exec as many as possible
// Before write back
int startExecALU(struct RS_line *this_RS, int cycles);

// ALU has a seat or not
int hasSeatALU(struct RS_line *this_RS);

//Whether all data in this_RS is ready
int dataReadyRS(struct RS_line *this_RS);

//ISSUE stage is complete (1 cycle has passed.)
int issueComplete(struct RS_line *this_RS, int cycles);

//execute tail of Load / Store Queue if possible: source is ready
int startExecLS(struct LsQueue_line *this_LSQ, int cycles);

//base address of this Load / Store Queue is ready
int baseAddrReadyLSQ(struct LsQueue_line *this_LSQ);



#endif
