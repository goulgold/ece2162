#include "writeback.h"
#include "timingtable.h"
#include "instr.h"
#include <stdlib.h>

extern struct ALU_ ALU;
extern struct RS_ RS;
extern struct ROB_ ROB;
extern struct LsQueue LSQ;


int exeCompleteALU(struct RS_line *this_RS, int cycles) {
    int alu_index = this_RS->alu_index;
    if (cycles - this_RS->cycles >= ALU.entity[alu_index].exec_cycle &&
        this_RS->stage == EXEC) {
        //TODO
        return TRUE;
    }
    return FALSE;
}

int startWback(struct RS_line *this_RS,
        int cycles) {
     //TODO
     //get result
    float result = getResultALU(this_RS);
    // update alu to free
    ALU.entity[this_RS->alu_index].busy = FALSE;
    // update ROB
    this_RS->dst->ttable_index = this_RS->ttable_index;
    this_RS->dst->val = result;
    this_RS->dst->finished = TRUE;
    // update timing table
    startWBtable(this_RS->ttable_index, cycles);
    // update this_RS to free
    this_RS->stage = WBACK;
    this_RS->cycles = cycles;
    this_RS->busy = FALSE;
    // update all other RS
    for (int i = 0; i < RS.size; ++i) {
        if (RS.entity[i].tag_1 == this_RS->dst) {
            RS.entity[i].val_1 = result;
            RS.entity[i].tag_1 = NULL;
        }
        if (RS.entity[i].tag_2 == this_RS->dst) {
            if (isSubInstr(RS.entity[i].instr_type)) {
                RS.entity[i].val_2 = (-1.0)*result;
            } else {
                RS.entity[i].val_2 = result;
            }
            RS.entity[i].tag_2 = NULL;
        }
    }
    //update all other LSQ
    for (int i = 0; i <  LSQ.size; ++i) {
        if (LSQ.entity[i].instr_type == SD && LSQ.entity[i].dst == this_RS->dst) {
            LSQ.entity[i].mem_val = result;
            LSQ.entity[i].dst = NULL;
        }
        if (LSQ.entity[i].tag_1 == this_RS->dst) {
            LSQ.entity[i].val_1 = (int) result;
            LSQ.entity[i].tag_1 = NULL;
        }
    }

    return TRUE;
}

float getResultALU(struct RS_line *this_RS) {
    //TODO
    int alu_type = this_RS->alu_type;
    float result;
    switch(alu_type) {
        case ALU_ADDI:
            result = this_RS->val_1 + this_RS->val_2;
            break;
        case ALU_MULI:
            result = this_RS->val_1 * this_RS->val_2;
            break;
        case ALU_DIVI:
            result = this_RS->val_1 / this_RS->val_2;
            break;
        case ALU_ADDD:
            result = this_RS->val_1 + this_RS->val_2;
            break;
        case ALU_MULD:
            result = this_RS->val_1 * this_RS->val_2;
            break;
        case ALU_DIVD:
            result = this_RS->val_1 / this_RS->val_2;
            break;
        default:
            result = 0.0;
            break;
    }
    return result;
}

int memCompleteLoad(struct LsQueue_line *this_LSQ, int cycles) {
    return (this_LSQ->stage == MEM &&
            this_LSQ->cycle - cycles >= this_LSQ->mem_cycle);
}

int startWbackLoad(struct LsQueue_line *this_LSQ, int cycles) {
    // update ROB
    float result = this_LSQ->mem_val;
    this_LSQ->dst->ttable_index = this_LSQ->ttable_index;
    this_LSQ->dst->val = result;
    this_LSQ->dst->finished = TRUE;
    //update timing table
    startWBtable(this_LSQ->ttable_index, cycles);
    //update LSQ to free
    this_LSQ->busy = FALSE;
    this_LSQ->finished = FALSE;
    this_LSQ->data_ready = FALSE;
    this_LSQ->stage = WBACK;
    //update all other RS
    for (int i = 0; i < RS.size; ++i) {
        if (RS.entity[i].tag_1 == this_LSQ->dst) {
            RS.entity[i].val_1 = result;
            RS.entity[i].tag_1 = NULL;
        }
        if (RS.entity[i].tag_2 == this_LSQ->dst) {
            if (isSubInstr(RS.entity[i].instr_type)) {
                RS.entity[i].val_2 = (-1.0)*result;
            } else {
                RS.entity[i].val_2 = result;
            }
            RS.entity[i].tag_2 = NULL;
        }
    }
    // update all other LSQ
    for (int i = 0; i < LSQ.size; ++i) {
        if (LSQ.entity[i].instr_type == SD && LSQ.entity[i].dst == this_LSQ->dst) {
            LSQ.entity[i].mem_val = this_LSQ->mem_val;
            LSQ.entity[i].data_ready = TRUE;
            LSQ.entity[i].dst = NULL;
        }
    }
    return TRUE;
}
