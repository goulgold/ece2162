#include "lower.h"

int getNum(char *string, int offset) {
    char s[10];
    int i = 0;
    while ((string[offset] > 47 && string[offset] < 58) || string[offset] == '-') {
        s[i] = string[offset];
        i++;
        offset++;
    }
    s[i] = '\0';
    return atoi(s);
}

float getNumf(char *string, int offset) {
    char s[10];
    int i = 0;
    while ((string[offset] > 47 && string[offset] < 58) || string[offset] == '.') {
        s[i] = string[offset];
        i++;
        offset++;
    }
    s[i] = '\0';
    return atof(s);

}

int getALUtype(char *line) {
    if (strstr(line, "Integer adder") != NULL)
        return ALU_ADDI;
    else if (strstr(line, "Integer multiplier") != NULL)
        return ALU_MULI;
    else if (strstr(line, "FP adder") != NULL)
        return ALU_ADDD;
    else if (strstr(line, "FP multiplier") != NULL)
        return ALU_MULD;
    else if (strstr(line, "Load/store unit") != NULL)
        return LOAD_STORE;
    else
        return -1;
}

int getRsnum(char *line) {
    return getNum(line, 16);
}

int getExcyc(char *line) {
    return getNum(line, 32);
}

int getMemcyc(char *line) {
    return getNum(line, 48);
}

int getFunum(char *line) {
    return getNum(line, 64);
}

void getIRF(char *line, int *int_RF) {
    char *ret;
    int index;
    int val;
    ret = strchr(line, 'R');
    while (ret != NULL) {
        ret++;
        index = getNum(ret, 0);
        ret = strchr(ret, '=');
        ret++;
        val = getNum(ret, 0);
        int_RF[index] = val;
        ret = strchr(ret, 'R');
        //printf("%s\n", ret);
    }
}

void getFRF(char *line, float *float_RF) {
    char *ret;
    int index;
    float val;
    ret = strchr(line, 'F');
    while (ret != NULL) {
        ret++;
        index = getNum(ret, 0);
        ret = strchr(ret, '=');
        ret++;
        val = getNumf(ret, 0);
        float_RF[index] = val;
        ret = strchr(ret, 'F');
    }
}

void getMem(char *line, float *data_mem) {
    char *ret;
    int index;
    float val;
    ret = strstr(line, "Mem");
    while (ret != NULL) {
        ret += 4;
        index = getNum(ret, 0);
        ret = strchr(ret, '=');
        ret++;
        val = getNumf(ret, 0);
        data_mem[index / 4] = val;
        ret = strstr(ret, "Mem");
    }
}

void getInstr(FILE *fp, struct input_instr *instr_mem) {
    char line[100];
    fgets(line, 100, fp);
    while (line[0] == '\n') {
        fgets(line, 100, fp);
    }
    int index = 0;
    do {
        getInstrline(line, instr_mem, index);
        index++;
    } while (fgets(line, 100, fp) != NULL);
}

int isFloatInstr(int op) {
    return (op == ADD_D ||
            op == SUB_D ||
            op == MULT_D ||
            op == LD ||
            op == SD
            );
}

void getInstrline(char *line, struct input_instr *instr_mem, int index) {
    struct input_instr * pinstr = &instr_mem[index];
    if (strstr(line, "ld ") != NULL)
        pinstr->op = LD;
    else if(strstr(line, "mult.d ") != NULL)
        pinstr->op = MULT_D;
    else if(strstr(line, "add.d ") != NULL)
        pinstr->op = ADD_D;
    else if(strstr(line, "sd ") != NULL)
        pinstr->op = SD;
    else if(strstr(line, "addi ") != NULL)
        pinstr->op = ADDI;
    else if(strstr(line, "bne ") != NULL)
        pinstr->op = BNE;
    else
        pinstr->op = -1;
    char *ret;
    int val;
    // obtain operand
    // destination
    ret = strchr(line, ' ');
    ret++;
    if (ret[0] == 'R') {
        ret++;
        val = getNum(ret, 0);
        pinstr->rd = val;
    } else {
        ret++;
        val = getNum(ret, 0);
        pinstr->rd = val + ARF_SIZE;
    }
    // source
    char *ret_backup = ret;
    if (pinstr->op > 30000) { // load/store
        ret = strchr(ret, 'R');
        ret++;
        val = getNum(ret, 0);
        pinstr->rs = val;
    } else if(isFloatInstr(pinstr->op)) {
        ret = strchr(ret, 'F');
        ret++;
        val = getNum(ret, 0);
        pinstr->rs = val + ARF_SIZE;
    } else {
        ret = strchr(ret, 'R');
        ret++;
        val = getNum(ret, 0);
        pinstr->rs = val;
    }
    // target
    if (pinstr->op > 30000) {
        ret = ret_backup;
        ret = strchr(ret, ' ');
        ret++;
        val = getNum(ret, 0);
        pinstr->rt = val;
    } else if (strchr(ret, 'R') != NULL) {
        ret = strchr(ret, 'R');
        ret++;
        val = getNum(ret, 0);
        pinstr->rt = val;
    } else if (strchr(ret, 'F') != NULL) {
         ret = strchr(ret, 'F');
         ret++;
         val = getNum(ret, 0);
         pinstr->rt = val + ARF_SIZE;
    } else {
         ret = strchr(ret, ' ');
         ret++;
         val = getNum(ret, 0);
         pinstr->rt = val;
    }
    pinstr->valid = 1;
}

int addRSROB(int alu_type,
             struct input_instr thisInstr,
             struct RS_line *RS,
             int RS_size,
             struct ROB_line *ROB,
             int ROB_size,
             int *ROB_nextfree,
             struct RAT_line *RAT,
             int PC,
             float *RF) {
    int result = FALSE;
    for (int i = 0; i < RS_size; ++i) {
        if (RS[i].alu_type == alu_type && RS[i].busy == FALSE) {

            // allocate RS instruction type
            RS[i].instr_type = thisInstr.op;
            RS[i].instr_addr = PC;

            // allocate RS source
            if (RAT[thisInstr.rs].tag == 0) {
                RS[i].val_1 = RF[thisInstr.rs];
                RS[i].tag_1 = NULL;
            } else {
                RS[i].tag_1 = RAT[thisInstr.rs].re_name;
            }

            // allocate RS target
            if (thisInstr.op == ADDI || thisInstr.op == SUBI) {
                RS[i].val_2 = thisInstr.rt;
                RS[i].tag_2 = NULL;
            } else {
                if (RAT[thisInstr.rt].tag == 0) {
                   RS[i].val_2 = RF[thisInstr.rt];
                   RS[i].tag_2 = NULL;
                } else {
                    RS[i].tag_2 = RAT[thisInstr.rt].re_name;
                }
            }

            // allocate RS destination & update RAT and ROB
            //TODO check ROB is full or not
            RS[i].dst = &ROB[*ROB_nextfree];
            RAT[thisInstr.rd].tag = 1;
            RAT[thisInstr.rd].re_name = RS[i].dst;
            ROB[*ROB_nextfree].dst = thisInstr.rd;
            *ROB_nextfree = (ROB_nextfree + 1) % ROB_size;
        }
    }
}
