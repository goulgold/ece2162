#include "upper.h"

int Parse_File(char *file_name,
           struct input_instr *instr_mem,
           float *data_mem,
           struct RS_line **RS,
           int *RS_size,
           float *float_RF,
           int *int_RF,
           struct ROB_line **ROB,
           int *ROB_size,
           struct ALU_line **ALU,
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
    *RS = (struct RS_line *) malloc( *RS_size * sizeof(struct RS_line));
    memset(*RS, 0, *RS_size * sizeof(struct RS_line));
    *ALU = (struct ALU_line *) malloc( *ALU_size * sizeof(struct ALU_line));
    int index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_rs[i]; j++) {
            (*RS)[index].instr_type = alu_type[i];
            index++;
        }
    }
    index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_fu[i]; ++j) {
            (*ALU)[index].type = alu_type[i];
            (*ALU)[index].exec_cycle = cyc_ex[i];
            (*ALU)[index].mem_cycle = cyc_mem[i];
        }
    }

    // start read ROB information
    fgets(line, 100, fp);
    *ROB_size = getNum(line, 14);
    *ROB = (struct ROB_line *) malloc(*ROB_size * sizeof(struct ROB_line));
    memset(*ROB, 0, *ROB_size * sizeof(struct ROB_line));
    for (int i = 0; i < *ROB_size; ++i) {
        (*ROB)[i].index = i+1;
    }

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

int printStatus(struct input_instr *instr_mem,
                float *data_mem,
                struct RS_line *RS,
                int RS_size,
                float *float_RF,
                int *int_RF,
                struct ROB_line *ROB,
                int ROB_size,
                struct ALU_line *ALU,
                int ALU_size,
                struct RAT_line *RAT) {
    //print instruction memory;
    printf("Instruction memory:\n");
    for (int i = 0; i < MEM_SIZE; ++i) {
        if (instr_mem[i].valid == 1) {
            printf("%d %d, %d, %d\n",
                    instr_mem[i].op,
                    instr_mem[i].rd,
                    instr_mem[i].rs,
                    instr_mem[i].rt);
        }
    }

    //print Data memory;
    printf("Data memory:\n");
    for (int i = 0; i < MEM_SIZE; ++i) {
            //printf("%08x\n",(int) data_mem[i]);
    }

    //print Register Station
    printf ("Register Station:\n");
    printf ("type\tbusy\taddr\tdst\ttag1\ttag2\tval1\tval2\tstage\tcycles\n");
    for (int i = 0; i < RS_size; ++i) {
        printf("%d\t%d\n", RS[i].instr_type, RS[i].busy);
    }

    //print Integer Register File (RF)
    for (int i = 0; i < ARF_SIZE; ++i) {
        printf("R%d: %d\n", i, int_RF[i]);
    }

    //print Float Register File (RF)
    for (int i = 0; i < ARF_SIZE; ++i) {
         printf("F%d: %f\n", i, float_RF[i]);
    }

    //print RAT
    //Integer
    printf ("IRAT:\n");
    for (int i = 0; i < ARF_SIZE; ++i) {
         if (RAT[i].tag == 0)
             printf("IRAT%d: RF\n", i);
         else
             printf("IRAT%d: ROB%d\n", i, RAT[i].re_name->index);
    }
    //Float
    printf ("FRAT:\n");
    for (int i = 0; i < ARF_SIZE; ++i) {
         if (RAT[i+ARF_SIZE].tag == 0)
             printf("FRAT%d: RF\n", i);
         else
             printf("FRAT%d: ROB%d\n", i, RAT[i+ARF_SIZE].re_name->index);
    }

    //Re-order Buffer (ROB)
    printf ("ROB:\n");
    printf ("index\tdes\tval\tfinished\n");
    for (int i = 0; i < ROB_size; ++i) {
        printf("%d\t", ROB[i].index);
        if (ROB[i].busy != 0) {
            printf("%d\t%f\t%d\n",
                    ROB[i].dst,
                    ROB[i].val,
                    ROB[i].finished);
        } else {
             printf("\n");
        }
    }



    return 0;
}


int has_instr(struct input_instr *instr_array, int PC) {
    if (instr_array[PC].valid == 1)
        return 1;
    else
        return 0;

}

int ROB_empty(struct ROB_line *ROB, int ROB_size) {
    int result = TRUE;
    for (int i = 0; i < ROB_size; ++i) {
        if (ROB[i].busy == TRUE)
            return FALSE;
    }
    return TRUE;
}

int instr2RS(struct input_instr *instr_mem,
             int *PC,
             struct RS_line *RS,
             int RS_size,
             struct ROB_line *ROB,
             int ROB_size,
             struct RAT_line *RAT,
             int *ROB_nextfree) {
    //TODO
}

int toExec(struct RS_line *RS,
           int RS_size,
           struct ALU_line *ALU,
           int ALU_size,
           struct ROB_line *ROB,
           int ROB_size) {
    //TODO
}

int toWback(struct RS_line *RS,
        int RS_size,
        struct ALU_line *ALU,
        int ALU_size,
        struct ROB_line *ROB,
        int ROB_size,
        int cdb_free) {
     //TODO
}

int toCommit(struct ROB_line *ROB,
             int *ROB_nextcommit,
             struct RAT_line *RAT,
             int *int_RF,
             float *float_RF) {
     //TODO
}

