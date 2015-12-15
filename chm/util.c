#include "util.h"
#include <string.h>
#include "instr.h"

int notFinishROB(struct ROB_line *this_ROB) {
    if (this_ROB->busy == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int isFloatInstr(int op) {
    return (op == ADD_D ||
            op == SUB_D ||
            op == MULT_D ||
            op == LD ||
            op == SD
            );
}

int isSubInstr(int op) {
    return (op == SUB ||
            op == SUB_D ||
            op == SUBI);
}


int instr2ALUtype(struct input_instr this_instr) {
    int result;
    int op = this_instr.op;
    switch(op) {
        case ADD:
            result = ALU_ADDI;
            break;
        case ADD_D:
            result = ALU_ADDD;
            break;
        case ADDI:
            result = ALU_ADDI;
            break;
        case SUB:
            result = ALU_ADDI;
            break;
        case SUB_D:
            result = ALU_ADDD;
            break;
        case SUBI:
            result = ALU_ADDD;
            break;
        case MULT:
            result = ALU_MULI;
            break;
        case MULT_D:
            result = ALU_MULD;
            break;
        case DIV:
            result = ALU_DIVI;
            break;
        case DIV_D:
            result = ALU_DIVD;
            break;
        case BEQ:
            result = ALU_ADDI;
            break;
        case BNE:
            result = ALU_ADDI;
            break;
        default:
            result = -1;
            break;
    }
    return result;
}


int resetRS(struct RS_line * this_RS) {
    int alu_type = this_RS->alu_type;
    memset(this_RS, 0, sizeof(struct RS_line));
    this_RS->alu_type = alu_type;
    return TRUE;
}
