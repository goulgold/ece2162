#ifndef COMMIT_H_
#define COMMIT_H_
#include "util.h"

int readyCommitROB(struct ROB_line this_ROB);

int startCommit(struct ROB_line *this_ROB,
             int cycles);

#endif
