#include "util.h"

// for store, the data is ready. for load , data is always ready
int dataReadyLSQ(struct LsQueue_line *this_LSQ);

//start memory. for store, write data in LSQ. for load, start mem back.
int startMemLSQ(struct LsQueue_line *this_LSQ, int cycles, int *membus_free);
