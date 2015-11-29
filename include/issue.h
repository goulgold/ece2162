#ifndef ISSUE_H_
#define ISSUE_H_
#include "util.h"

//PC pointer's location, has a instr or not
int has_instr(int PC);

// issue instr into RS, update RS, ROB, RAT, ALU, TimingTable
int issueALU(struct input_instr this_instr, int cycles);

// issue instr into LSQ, update ROB, RAT, TimingTable
int issueLS(struct input_instr this_instr, int cycles);

// this instruction is normal or not.
int isALUIns(struct input_instr this_instr);

// this instruction is load store instruction
int isLSIns(struct input_instr this_instr);

// RS has seat or not
int hasSeatRS(struct input_instr this_instr);

// ROB has seat or not
int hasSeatROB();

// LSQ has seat or not
int hasSeatLSQ();
#endif
