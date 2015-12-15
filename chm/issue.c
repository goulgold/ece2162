#include "issue.h"
#include "instr.h"
#include "timingtable.h"
#include <stdlib.h>
#include <string.h>

extern struct timetable_line *TimeTable;
extern int table_index;

extern struct input_instr *instr_mem;
extern struct RS_ RS;
extern struct LsQueue LSQ;
extern struct ROB_ ROB;
extern struct RAT_line *RAT;
extern int *int_RF;
extern float *float_RF;


int has_instr(int PC) {
    if (instr_mem[PC].valid == 1)
        return 1;
    else
        return 0;

}

int isALUIns(struct input_instr this_instr) {
    return (this_instr.op < BEQ);
}

int isBranchIns(struct input_instr this_instr){
    return (this_instr.op == BNE || this_instr.op == BEQ);
}

int isLSIns(struct input_instr this_instr) {
    return (this_instr.op > 30000 && this_instr.op < 40000);
}

int issueALU(struct input_instr this_instr, int cycles) {
    int seatRS = hasSeatRS(this_instr);

    //can't issue
    if (seatRS == -1 || !hasSeatROB(ROB))
        return FALSE;

    // update timing table
    startISSUEtable(table_index, cycles);
    RS.entity[seatRS].ttable_index = table_index;
    TimeTable[table_index].index = table_index;
    strcpy(TimeTable[table_index].instr_line, this_instr.instr_line);
    table_index++;

    // allocate RS information
    RS.entity[seatRS].instr_type = this_instr.op;
    RS.entity[seatRS].busy = TRUE;
    RS.entity[seatRS].stage = ISSUE;
    RS.entity[seatRS].cycles = cycles;

    // allocate RS source
    if (RAT[this_instr.rs].tag == 0) {
        if (isFloatInstr(this_instr.op)) {
            RS.entity[seatRS].val_1 = float_RF[this_instr.rs-ARF_SIZE];
        } else {
            RS.entity[seatRS].val_1 = int_RF[this_instr.rs];
        }
        RS.entity[seatRS].tag_1 = NULL;
    } else {
        RS.entity[seatRS].tag_1 = RAT[this_instr.rs].re_name;
    }

    // allocate RS target
    if (this_instr.op == ADDI || this_instr.op == SUBI) {
        if (isSubInstr(this_instr.op)) {
            RS.entity[seatRS].val_2 = (-1.0)*this_instr.rt;
            RS.entity[seatRS].tag_2 = NULL;
        } else {
            RS.entity[seatRS].val_2 = this_instr.rt;
            RS.entity[seatRS].tag_2 = NULL;
        }
    } else {
        if (RAT[this_instr.rt].tag == 0) {
            if (isFloatInstr(this_instr.op)) {
                if (isSubInstr(this_instr.op)) {
                    RS.entity[seatRS].val_2 = (-1.0)*float_RF[this_instr.rt-ARF_SIZE];
                } else {
                    RS.entity[seatRS].val_2 = float_RF[this_instr.rt-ARF_SIZE];
                }

            } else {
                if (isSubInstr(this_instr.op)) {
                    RS.entity[seatRS].val_2 = (-1.0)*int_RF[this_instr.rt];
                } else {
                    RS.entity[seatRS].val_2 = int_RF[this_instr.rt];
                }
            }
            RS.entity[seatRS].tag_2 = NULL;
        } else {
            RS.entity[seatRS].tag_2 = RAT[this_instr.rt].re_name;
        }
    }

    // allocate RS destination
    RS.entity[seatRS].dst = &ROB.entity[ROB.nextfree];
    // update RAT and ROB
    RAT[this_instr.rd].tag = 1;
    RAT[this_instr.rd].re_name = RS.entity[seatRS].dst;
    ROB.entity[ROB.nextfree].dst = this_instr.rd;
    ROB.entity[ROB.nextfree].busy = TRUE;

    // update tag_1 and tag_2 if data is ready
    struct RS_line *this_RS = &RS.entity[seatRS];
    if (this_RS->tag_1 != NULL && this_RS->tag_1->finished == TRUE) {
         float val = this_RS->tag_1->val;
         if (isSubInstr(this_RS->instr_type)) {
             this_RS->val_1 = (-1.0)*val;
             this_RS->tag_1 = NULL;
         } else {
             this_RS->val_1 = val;
             this_RS->tag_1 = NULL;
         }
    }
    if (this_RS->tag_2 != NULL && this_RS->tag_2->finished == TRUE) {
         float val = this_RS->tag_2->val;
         if (isSubInstr(this_RS->instr_type)) {
             this_RS->val_2 = (-1.0)*val;
             this_RS->tag_2 = NULL;
         } else {
             this_RS->val_2 = val;
             this_RS->tag_2 = NULL;
         }
    }

    return TRUE;
}

uint32_t prediction(struct input_instr this_instr, int seatRS)
{
    uint32_t branch_pc = PC;
    if((btb[BTB_IDX(branch_pc)].tag != branch_pc) ||
            (btb[BTB_IDX(branch_pc)].valid == 0)){
        RS.entity[seatRS].predicted_dest = branch_pc + 1;
        return 0;
    }else if((btb[BTB_IDX(branch_pc)].tag == branch_pc) &&
            (btb[BTB_IDX(branch_pc)].valid == 1)){
        RS.entity[seatRS].predicted_dest = btb[BTB_IDX(branch_pc)].target;
        return 1;
    }
    RS.entity[seatRS].predicted_dest = branch_pc + 1;
    return 0;
}

int issueBranch(struct input_instr this_instr, int cycles) {
    int seatRS = hasSeatRS(this_instr);

    //can't issue
    if (seatRS == -1 || !hasSeatROB(ROB))
        return FALSE;

    // update timing table
    startISSUEtable(table_index, cycles);
    RS.entity[seatRS].ttable_index = table_index;
    TimeTable[table_index].index = table_index;
    strcpy(TimeTable[table_index].instr_line, this_instr.instr_line);
    table_index++;

    // allocate RS information
    RS.entity[seatRS].instr_type = this_instr.op;
    RS.entity[seatRS].busy = TRUE;
    RS.entity[seatRS].stage = ISSUE;
    RS.entity[seatRS].cycles = cycles;

    // allocate RS source
    if (RAT[this_instr.rd].tag == 0) {
        RS.entity[seatRS].val_1 = int_RF[this_instr.rd];
        RS.entity[seatRS].tag_1 = NULL;
    } else {
        RS.entity[seatRS].tag_1 = RAT[this_instr.rd].re_name;
    }

    if (RAT[this_instr.rs].tag == 0) {
        RS.entity[seatRS].val_2 = int_RF[this_instr.rs];
        RS.entity[seatRS].tag_2 = NULL;
    } else {
        RS.entity[seatRS].tag_2 = RAT[this_instr.rs].re_name;
    }

    // update ROB commit marker
    if(pipe.branch_index == 0){
        pipe.branch_index = ROB.nextfree;
    }else{
        pipe.branch_unresovled = ROB.nextfree;
    }
    // update branch information
    RS.entity[seatRS].offset = this_instr.rs;
    // branch prediction
    RS.entity[seatRS].predicted_dir = prediction(this_instr, seatRS);
    RS.entity[seatRS].pc = PC;
    PC = RS.entity[seatRS].predicted_dest;

    // update tag_1 and tag_2 if data is ready
    struct RS_line *this_RS = &RS.entity[seatRS];
    if (this_RS->tag_1 != NULL && this_RS->tag_1->finished == TRUE) {
         float val = this_RS->tag_1->val;
         if (isSubInstr(this_RS->instr_type)) {
             this_RS->val_1 = (-1.0)*val;
             this_RS->tag_1 = NULL;
         } else {
             this_RS->val_1 = val;
             this_RS->tag_1 = NULL;
         }
    }
    if (this_RS->tag_2 != NULL && this_RS->tag_2->finished == TRUE) {
         float val = this_RS->tag_2->val;
         if (isSubInstr(this_RS->instr_type)) {
             this_RS->val_2 = (-1.0)*val;
             this_RS->tag_2 = NULL;
         } else {
             this_RS->val_2 = val;
             this_RS->tag_2 = NULL;
         }
    }

    return TRUE;
}

int issueLS(struct input_instr this_instr, int cycles) {
// TODO
// can issue or not
    int seatLSQ = hasSeatLSQ();
    if (hasSeatLSQ() == -1 || !hasSeatROB()) {
        return FALSE;
    }
// update timing table
    startISSUEtable(table_index, cycles);
    LSQ.entity[seatLSQ].ttable_index = table_index;
    TimeTable[table_index].index = table_index;
    strcpy(TimeTable[table_index].instr_line, this_instr.instr_line);
    table_index++;

    // update LSQ
    LSQ.entity[seatLSQ].busy = TRUE;
    LSQ.entity[seatLSQ].finished = FALSE;
    LSQ.entity[seatLSQ].instr_type = this_instr.op;
    LSQ.entity[seatLSQ].stage = ISSUE;
    LSQ.entity[seatLSQ].cycle = cycles;

    // for both LOAD and STORE instruction

    // update LSQ source
    if (RAT[this_instr.rs].tag == 0)
        LSQ.entity[seatLSQ].val_1 = int_RF[this_instr.rs];
    else if (RAT[this_instr.rs].re_name->finished == TRUE)
        LSQ.entity[seatLSQ].val_1 = RAT[this_instr.rs].re_name->val;
    else
        LSQ.entity[seatLSQ].tag_1 = RAT[this_instr.rs].re_name;
    // update LSQ target
    LSQ.entity[seatLSQ].offset = this_instr.rt;

    //updat ROB
    ROB.entity[ROB.nextfree].busy = TRUE;
    ROB.entity[ROB.nextfree].finished = FALSE;


    // for STORE instruction, if destination data is ready, update ROB to finished in write back
    // don't update RAT
    if (this_instr.op == SD) {
        ROB.entity[ROB.nextfree].store_instr = TRUE;
        ROB.entity[ROB.nextfree].busy = TRUE;
        LSQ.entity[seatLSQ].buffer = &ROB.entity[ROB.nextfree];
        if (RAT[this_instr.rd].tag == 0) {
            LSQ.entity[seatLSQ].mem_val = float_RF[this_instr.rd - ARF_SIZE];
            LSQ.entity[seatLSQ].data_ready = TRUE;
        } else {
            LSQ.entity[seatLSQ].dst = RAT[this_instr.rd].re_name;
            LSQ.entity[seatLSQ].data_ready = FALSE;
        }
    }
    //for LOAD instruction, like all other ALU instruction
    if (this_instr.op == LD) {
        // update LSQ destination
        LSQ.entity[seatLSQ].dst = &ROB.entity[ROB.nextfree];
        // update ROB & RAT
        RAT[this_instr.rd].tag = 1;
        RAT[this_instr.rd].re_name = LSQ.entity[seatLSQ].dst;
        ROB.entity[ROB.nextfree].dst = this_instr.rd;
        ROB.entity[ROB.nextfree].busy = TRUE;
        ROB.entity[ROB.nextfree].finished = FALSE;
    }

    return TRUE;
}

int hasSeatRS(struct input_instr this_instr) {
    int this_alutype = instr2ALUtype(this_instr);
    for (int i = 0; i < RS.size; ++i) {
        if (RS.entity[i].alu_type == this_alutype && RS.entity[i].busy == FALSE) {
            return i;
        }
    }
    return -1;
}

int hasSeatROB() {
    if (ROB.entity[ROB.nextfree].busy == FALSE)
        return TRUE;
    else
        return FALSE;
}

int hasSeatLSQ() {
    if (!LSQ.entity[LSQ.head].busy) {
        return LSQ.head;
    } else {
        return -1;
    }
}

