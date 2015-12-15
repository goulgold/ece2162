#ifndef TIMETABLE_H_
#define TIMETABLE_H_

#define MAX_TTABLE_LINE 100 // timing table size

struct timetable_line {
    char instr_line[1024];
    int index; // index # in table
    int issue; // cycle start issue
    int exec; // cycle start exec
    int mem; // cycle start mem
    int wb; // cycle start write back
    int commit; // cycle start commit
};

int startISSUEtable(int index, int cycle);
int startEXECtable(int index, int cycle);
int startMEMtable(int index, int cycle);
int startWBtable(int index, int cycle);
int startCOMMITtable(int index, int cycle);
int printTimetable();

#endif
