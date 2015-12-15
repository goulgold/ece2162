#include "parser.h"
#include "instr.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern struct input_instr *instr_mem;
extern float *data_mem;
extern struct RS_ RS;
extern int *int_RF;
extern float *float_RF;
extern struct ROB_ ROB;
extern struct LsQueue LSQ;
extern struct ALU_ ALU;

int Parse_File(char *input_file_name,
               char *conf_file_name) {
    // Open global conf file
    FILE *fp;
    fp = fopen(conf_file_name, "r");
    if (fp == NULL) return -1;

    //ignore the first line.
    char line[100];
    fgets(line, 100, fp);

    //start read ALU information
    RS.size = 0;
    ALU.size = 0;
    fgets(line, 100, fp);
    int alu_type[MAX_ALU];
    int num_rs[MAX_ALU];
    int cyc_ex[MAX_ALU];
    int cyc_mem[MAX_ALU];
    int num_fu[MAX_ALU];

    // Load Store Queue
    int ls_num;
    int ls_cyc_ex;
    int ls_cyc_mem;
    int ls_fu;
    int i = 0;
    while (line[0] != '\n') {
        if (isALUunit(getALUtype(line))) { // normal ALU unit
            alu_type[i] = getALUtype(line);
            num_rs[i] = getRsnum(line);
            cyc_ex[i] = getExcyc(line);
            cyc_mem[i] = getMemcyc(line);
            num_fu[i] = getFunum(line);
            RS.size += num_rs[i];
            ALU.size += num_fu[i];
            i++;
            fgets(line, 100, fp);
        } else { // Load / Store unit
            ls_num = getRsnum(line);
            ls_cyc_ex = getExcyc(line);
            ls_cyc_mem = getMemcyc(line);
            ls_fu = getFunum(line);
            fgets(line, 100, fp);
        }
    }
    int kind_alu = i;
    // initialize RS
    RS.entity = (struct RS_line *) malloc( RS.size * sizeof(struct RS_line));
    memset(RS.entity, 0, RS.size * sizeof(struct RS_line));
    int index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_rs[i]; j++) {
            RS.entity[index].alu_type = alu_type[i];
            index++;
        }
    }

    // initialize Load Store Queue
    index = 0;
    LSQ.entity = malloc(ls_num * sizeof(struct LsQueue_line));
    for (int i = 0; i < ls_num; ++i) {
        LSQ.entity[i].alu_type = LOAD_STORE;
        LSQ.entity[i].exec_cycle = ls_cyc_ex;
        LSQ.entity[i].mem_cycle = ls_cyc_mem;
    }
    LSQ.size = ls_num;
    LSQ.head = 0;
    LSQ.tail = 0;

    // initialize ALU
    ALU.size += ls_fu;
    ALU.entity = (struct ALU_line *) malloc(ALU.size * sizeof(struct ALU_line));
    index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_fu[i]; ++j) {
            ALU.entity[index].type = alu_type[i];
            ALU.entity[index].exec_cycle = cyc_ex[i];
            ALU.entity[index].mem_cycle = cyc_mem[i];
            index++;
        }
    }
    for (int i = 0; i < ls_fu; ++i) {
         ALU.entity[index].type = LOAD_STORE;
         ALU.entity[index].exec_cycle = ls_cyc_ex;
         ALU.entity[index].mem_cycle = ls_cyc_mem;
         index++;
    }

    // start read ROB information
    fgets(line, 100, fp);
    ROB.size = getNum(line, 14);
    ROB.entity = (struct ROB_line *) malloc(ROB.size * sizeof(struct ROB_line));
    memset(ROB.entity, 0, ROB.size * sizeof(struct ROB_line));
    for (int i = 0; i < ROB.size; ++i) {
        ROB.entity[i].index = i+1;
    }
    ROB.nextfree = 0;
    fclose(fp);

    /********************
     * read input file
     * *****************/

    fp = fopen(input_file_name, "r");
    if (fp == NULL) return -1;

    // instrution index
    index = 0;

    while (fgets(line, 100, fp) != NULL) {
        if (!getRF(line, int_RF, float_RF) && !getMem(line, data_mem)) {
            if(getInstrline(line, instr_mem, index))
                index++;
        }
        memset(line, 0, 100);
    }

return 0;
}

int getNum(char *string, int offset) {
    char s[10];
    int i = 0;
    while (string[offset] == ' ') offset++;
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
    while (string[offset] == ' ') offset++;
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

int isALUunit(int alu_type) {
    if (alu_type != LOAD_STORE)
        return TRUE;
    else
        return FALSE;
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

int getRF(char *line, int *int_RF, float *float_RF) {
    if (line[0] == 'R' || line[0] == 'F') {
        getIRF(line, int_RF);
        getFRF(line, float_RF);
    } else {
        return FALSE;
    }
    return TRUE;
}

int getIRF(char *line, int *int_RF) {
    char *ret;
    int index;
    int val;
    ret = strchr(line, 'R');
    if (!ret) return FALSE;
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
    return TRUE;
}

int getFRF(char *line, float *float_RF) {
    char *ret;
    int index;
    float val;
    ret = strchr(line, 'F');
    if (!ret) return FALSE;
    while (ret != NULL) {
        ret++;
        index = getNum(ret, 0);
        ret = strchr(ret, '=');
        ret++;
        val = getNumf(ret, 0);
        float_RF[index] = val;
        ret = strchr(ret, 'F');
    }
    return TRUE;
}

int getMem(char *line, float *data_mem) {
    char *ret;
    int index;
    float val;
    ret = strstr(line, "Mem");
    if (!ret) return FALSE;
    while (ret != NULL) {
        ret += 4;
        index = getNum(ret, 0);
        ret = strchr(ret, '=');
        ret++;
        val = getNumf(ret, 0);
        data_mem[index / 4] = val;
        ret = strstr(ret, "Mem");
    }
    return TRUE;
}

int getInstrline(char *line, struct input_instr *instr_mem, int index) {
    struct input_instr * pinstr = &instr_mem[index];
    strcpy(pinstr->instr_line, line);
    if (strstr(line, "ld ") != NULL)
        pinstr->op = LD;
    else if(strstr(line, "add ") != NULL)
        pinstr->op = ADD;
    else if(strstr(line, "addi ") != NULL)
        pinstr->op = ADDI;
    else if(strstr(line, "add.d ") != NULL)
        pinstr->op = ADD_D;
    else if(strstr(line, "sub ") != NULL)
        pinstr->op = SUB;
    else if(strstr(line, "subi ") != NULL)
        pinstr->op = SUBI;
    else if(strstr(line, "sub.d ") != NULL)
        pinstr->op = SUB_D;
    else if(strstr(line, "mult ") != NULL)
        pinstr->op = MULT;
    else if(strstr(line, "mult.d ") != NULL)
        pinstr->op = MULT_D;
    else if(strstr(line, "sd ") != NULL)
        pinstr->op = SD;
    else if(strstr(line, "bne ") != NULL)
        pinstr->op = BNE;
    else if(strstr(line, "beq ") != NULL)
        pinstr->op = BEQ;
    else
        return FALSE;
    char *ret;
    int val;
    // obtain operand
    // destination
    ret = strchr(line, ' ');
    while (ret[0] == ' ')
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
    // ld/store
    if (pinstr->op > 30000) { // load/store
        ret = ret_backup;
        ret = strchr(ret, ' ');
        ret++;
        val = getNum(ret, 0);
        pinstr->rt = val;
    } else { // ALU instr
        ret = strchr(ret, ' ');
        while (*ret == ' ')
            ret++;
        if (*ret == 'R') {
            ret++;
            val = getNum(ret, 0);
            pinstr->rt = val;
        } else if (*ret == 'F') {
            ret++;
            val = getNumf(ret, 0);
            pinstr->rt = val + ARF_SIZE;
        } else {
            val = getNum(ret, 0);
            pinstr->rt = val;
        }
    }
    pinstr->valid = 1;
    return TRUE;
}

