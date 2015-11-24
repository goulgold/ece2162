#include "timingtable.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

struct timetable_line *TimeTable;
int table_index = 0;

int startISSUEtable(int index, int cycle) {
    TimeTable[index].issue = cycle;
    return TRUE;
}
int startEXECtable(int index, int cycle) {
    TimeTable[index].exec = cycle;
    return TRUE;
}
int startMEMtable(int index, int cycle) {
    TimeTable[index].mem = cycle;
    return TRUE;
}
int startWBtable(int index, int cycle) {
    TimeTable[index].wb = cycle;
    return TRUE;
}
int startCOMMITtable(int index, int cycle) {
    TimeTable[index].commit = cycle;
    return TRUE;
}
int printTimetable() {
    printf ("index\tissue\texec\tmem\twb\tcommit\n");
    for (int i = 0; i < MAX_TTABLE_LINE; ++i) {
        if (TimeTable[i].issue != 0) {
            printf("%d\t%d\t%d\t%d\t%d\t%d\n",
                    TimeTable[i].index,
                    TimeTable[i].issue,
                    TimeTable[i].exec,
                    TimeTable[i].mem,
                    TimeTable[i].wb,
                    TimeTable[i].commit);
        }
    }
    return TRUE;
}
