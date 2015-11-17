#include "upper.h"

int Parse_File(char *file_name,
           struct input_instr *instr_mem,
           float *data_mem,
           struct RS_line *RS,
           int *RS_size,
           float *float_RF,
           int *int_RF,
           struct ROB_line *ROB,
           int *ROB_size,
           struct ALU_line *ALU,
           int *ALU_size) {
    // Open file
    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == NULL) return -1;

    //ignore the first line.
    char line[100];
    fgets(line, 100, fp);

    //start read ALU information
    *RS_size = 0;
    *ALU_size = 0;
    fgets(line, 100, fp);
    int alu_type[MAX_ALU];
    int num_rs[MAX_ALU];
    int cyc_ex[MAX_ALU];
    int cyc_mem[MAX_ALU];
    int num_fu[MAX_ALU];
    int i = 0;
    while (line[0] != '\n') {
        alu_type[i] = getALUtype(line);
        num_rs[i] = getRsnum(line);
        cyc_ex[i] = getExcyc(line);
        cyc_mem[i] = getMemcyc(line);
        num_fu[i] = getFunum(line);
        *RS_size += num_rs[i];
        *ALU_size += num_fu[i];
        i++;
        fgets(line, 100, fp);
    }
    int kind_alu = i;
    // initialize RS
    RS = (struct RS_line *) malloc( *RS_size * sizeof(struct RS_line));
    ALU = (struct ALU_line *) malloc( *ALU_size * sizeof(struct ALU_line));
    int index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_rs[i]; j++) {
            RS[index].instr_type = alu_type[i];
            index++;
        }
    }
    index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_fu[i]; ++j) {
            ALU[index].type = alu_type[i];
            ALU[index].exec_cycle = cyc_ex[i];
            ALU[index].mem_cycle = cyc_mem[i];
        }
    }

    // start read ROB information
    fgets(line, 100, fp);
    *ROB_size = getNum(line, 14);
    ROB = (struct ROB_line *) malloc(*ROB_size * sizeof(struct ROB_line));

    //start read Integer RF & Float RF
    fgets(line, 100, fp);
    getIRF(line, int_RF);
    getFRF(line, float_RF);

    //start read Memory information
    fgets(line, 100, fp);
    getMem(line, data_mem);

    //start read instruction information
    getInstr(fp, instr_mem);



return 0;
};

