#include <stdio.h>
#include "util.h"
#include "parser.h"
#include "instr.h"

int main(int argc, char **argv) {

    // 1.test arguments is valid or not
    //
    // 2.parse arguments load instr to memory
    //
    // 3.initialize： RS, ROB, ARF, Timing Table, Memory
    //
    // 4.Start simulate until PC point to NULL
    //
    // 4.1 if any RS is free, load a instr.
    // 4.2 send new cycle to all instrs in RS.
    // 4.3 update RS, ROB, ARF, Timing Table,
    // 4.4 commit in order
    return 0;
}

