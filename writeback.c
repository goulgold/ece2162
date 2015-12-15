#include "writeback.h"
#include "timingtable.h"
#include "instr.h"
#include <stdlib.h>

extern struct ALU_ ALU;
extern struct RS_ RS;
extern struct ROB_ ROB;
extern struct LsQueue LSQ;
extern struct pipe_state pipe;
extern buffer_t btb[BTB_SIZE];
extern int stall_commit;
extern int stall_issue;

void pipe_recover(uint32_t dest)
{
        if (pipe.branch_recover) return;

            /* schedule the recovery. This will be done once all pipeline stages simulate the current cycle. */
            pipe.branch_recover = 1;
            pipe.branch_dest = dest;

}



int exeCompleteALU(struct RS_line *this_RS, int cycles) {
    int alu_index = this_RS->alu_index;
    if (cycles - this_RS->cycles >= ALU.entity[alu_index].exec_cycle &&
        this_RS->stage == EXEC) {
        //TODO
        return TRUE;
    }
    return FALSE;
}

void update_branch_predictions(uint32_t branch_pc, uint8_t taken, uint32_t target)
{
    //update btb
    btb[BTB_IDX(branch_pc)].valid = 1;
    btb[BTB_IDX(branch_pc)].tag = branch_pc;
    btb[BTB_IDX(branch_pc)].target = target;
}

int startWback(struct RS_line *this_RS,
        int cycles) {
     //TODO
     //get result
    float result = getResultALU(this_RS);
    if(this_RS->is_branch == 1){
        this_RS->branch_taken = (int) result;

        /* handle branch recoveries at this point */
        if (((this_RS->predicted_dir != this_RS->branch_taken) ||
                ((btb[BTB_IDX(this_RS->pc)].valid == 0) ||
                (btb[BTB_IDX(this_RS->pc)].tag != this_RS->pc)))){
            stall_issue = TRUE;
            if (this_RS->branch_taken == 1)
                pipe_recover(this_RS->branch_dest);
            else
                pipe_recover(this_RS->pc + 1);
        } else {
            pipe.branch_index = -1;
            stall_commit = FALSE;
        }

            update_branch_predictions(this_RS->pc, this_RS->branch_taken, this_RS->branch_dest);
    }

    // update alu to free
    ALU.entity[this_RS->alu_index].busy = FALSE;
    // update ROB
    if (this_RS->dst) {
        this_RS->dst->ttable_index = this_RS->ttable_index;
        this_RS->dst->val = result;
        this_RS->dst->finished = TRUE;
    }
    // update timing table
    startWBtable(this_RS->ttable_index, cycles);
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
            LSQ.entity[i].data_ready = TRUE;
            LSQ.entity[i].dst = NULL;
        }
        if (LSQ.entity[i].tag_1 == this_RS->dst) {
            LSQ.entity[i].val_1 = (int) result;
            LSQ.entity[i].tag_1 = NULL;
        }
    }
    // update this_RS to free
    resetRS(this_RS);

    return TRUE;
}

float getResultALU(struct RS_line *this_RS) {
    //TODO
    int alu_type = this_RS->alu_type;
    float result;
    switch(alu_type) {
        case ALU_ADDI:
            if (this_RS->instr_type == BEQ) {
                result = this_RS->val_1 == this_RS->val_2 ? 1 : 0;
            } else if (this_RS->instr_type == BNE) {
                result = this_RS->val_1 != this_RS->val_2 ? 1 : 0;
            } else
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
            cycles - this_LSQ->cycle >= this_LSQ->mem_cycle);
}

int startWbackLoad(struct LsQueue_line *this_LSQ, int cycles) {
    // update ROB
    float result = this_LSQ->mem_val;
    this_LSQ->dst->ttable_index = this_LSQ->ttable_index;
    this_LSQ->dst->val = result;
    this_LSQ->dst->finished = TRUE;
    //update timing table
    startWBtable(this_LSQ->ttable_index, cycles);
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
            LSQ.entity[i].buffer->val = this_LSQ->mem_val;
            LSQ.entity[i].buffer->finished = TRUE;
            LSQ.entity[i].data_ready = TRUE;
            LSQ.entity[i].dst = NULL;
        }
    }
    //update LSQ to free
    resetLSQ_line(this_LSQ);
    return TRUE;
}
