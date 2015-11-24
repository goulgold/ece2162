#include "upper.h"
#include "timingtable.h"

extern struct timetable_line *TimeTable;
extern int table_index;

int Parse_File(char *input_file_name,
               char *conf_file_name,
               struct input_instr *instr_mem,
               float *data_mem,
               struct RS_ *RS,
               int *int_RF,
               float *float_RF,
               struct ROB_ *ROB,
               struct ALU_ *ALU) {
    // Open global conf file
    FILE *fp;
    fp = fopen(conf_file_name, "r");
    if (fp == NULL) return -1;

    //ignore the first line.
    char line[100];
    fgets(line, 100, fp);

    //start read ALU information
    RS->size = 0;
    ALU->size = 0;
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
        RS->size += num_rs[i];
        ALU->size += num_fu[i];
        i++;
        fgets(line, 100, fp);
    }
    int kind_alu = i;
    // initialize RS
    RS->entity = (struct RS_line *) malloc( RS->size * sizeof(struct RS_line));
    memset(RS->entity, 0, RS->size * sizeof(struct RS_line));
    ALU->entity = (struct ALU_line *) malloc(ALU->size * sizeof(struct ALU_line));
    int index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_rs[i]; j++) {
            RS->entity[index].alu_type = alu_type[i];
            index++;
        }
    }
    index = 0;
    for (int i = 0; i < kind_alu; ++i) {
        for (int j = 0; j < num_fu[i]; ++j) {
            ALU->entity[index].type = alu_type[i];
            ALU->entity[index].exec_cycle = cyc_ex[i];
            ALU->entity[index].mem_cycle = cyc_mem[i];
            index++;
        }
    }

    // start read ROB information
    fgets(line, 100, fp);
    ROB->size = getNum(line, 14);
    ROB->entity = (struct ROB_line *) malloc(ROB->size * sizeof(struct ROB_line));
    memset(ROB->entity, 0, ROB->size * sizeof(struct ROB_line));
    for (int i = 0; i < ROB->size; ++i) {
        ROB->entity[i].index = i+1;
    }
    ROB->nextfree = 0;
    fclose(fp);

    fp = fopen(input_file_name, "r");
    if (fp == NULL) return -1;

    //start read Integer RF & Float RF
    fgets(line, 100, fp);
    if (!getIRF(line, int_RF) && !getFRF(line, float_RF)) {
        fseek(fp, 0, SEEK_SET);
    }

    //start read Memory information
    fgets(line, 100, fp);
    if (!getMem(line, data_mem)) {
        fseek(fp, 0, SEEK_SET);
    }

    //start read instruction information
    getInstr(fp, instr_mem);

return 0;
};

int printStatus(struct input_instr *instr_mem,
                float *data_mem,
                struct RS_ *RS,
                float *float_RF,
                int *int_RF,
                struct ROB_ *ROB,
                struct ALU_ *ALU,
                struct RAT_line *RAT,
                int cycles) {
    //print cycles
    printf("cycles: %d\n", cycles);

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
    /*
    printf("Data memory:\n");
    for (int i = 0; i < MEM_SIZE; ++i) {
        printf("%08x\n",(int) data_mem[i]);
    }
    */

    //print Reservation Station
    /*
    printf ("Reservation Station:\n");
    printf ("type\tbusy\tdst\ttag1\ttag2\tval1\tval2\tcycles\tstage\n");
    for (int i = 0; i < RS->size; ++i) {
        printf("%d\t%d\t", RS->entity[i].instr_type, RS->entity[i].busy);
        printPointROB(RS->entity[i].dst);
        printPointROB(RS->entity[i].tag_1);
        printPointROB(RS->entity[i].tag_2);
        printf("%f\t", RS->entity[i].val_1);
        printf("%f\t", RS->entity[i].val_2);
        printf("%d\t", RS->entity[i].cycles);
        printf("%d\t", RS->entity[i].stage);
        printf("\n");

    }
    */

    //print Integer Register File (RF)
    printf("Register File:\n");
    for (int i = 0; i < ARF_SIZE; ++i) {
        printf("R%d: %d\t", i, int_RF[i]);
        if ((i+1) % MAX_DISPLAY == 0)
            printf("\n");
    }

    //print Float Register File (RF)
    for (int i = 0; i < ARF_SIZE; ++i) {
        printf("F%d: %f\t", i, float_RF[i]);
        if ((i+1) % MAX_DISPLAY == 0)
            printf("\n");
    }

    //print RAT
    //Integer
    /*
    printf ("IRAT:\n");
    for (int i = 0; i < ARF_SIZE; ++i) {
        if (RAT[i].tag == 0)
            printf("IRAT%d: RF\t", i);
        else
            printf("IRAT%d: ROB%d\t", i, RAT[i].re_name->index);
        if ((i+1) % MAX_DISPLAY == 0)
            printf("\n");
    }
    //Float
    printf ("FRAT:\n");
    for (int i = 0; i < ARF_SIZE; ++i) {
        if (RAT[i+ARF_SIZE].tag == 0)
            printf("FRAT%d: RF\t", i);
        else
            printf("FRAT%d: ROB%d\t", i, RAT[i+ARF_SIZE].re_name->index);
        if ((i+1) % MAX_DISPLAY == 0)
            printf("\n");
    }
    */

    //Re-order Buffer (ROB)
    /*
    printf ("ROB:\n");
    printf ("index\tbusy\tdes\tval\tfinished\n");
    for (int i = 0; i < ROB->size; ++i) {
        printf("%d\t", ROB->entity[i].index);
            printf("%d\t%d\t%f\t%d\n",
                    ROB->entity[i].busy,
                    ROB->entity[i].dst,
                    ROB->entity[i].val,
                    ROB->entity[i].finished);
            printf("\n");
    }
    */
    return 0;
}


int has_instr(struct input_instr *instr_array, int PC) {
    if (instr_array[PC].valid == 1)
        return 1;
    else
        return 0;

}

int ROB_empty(struct ROB_line *ROB, int ROB_size) {
    for (int i = 0; i < ROB_size; ++i) {
        if (ROB[i].busy == TRUE)
            return FALSE;
    }
    return TRUE;
}

int isALUIns(struct input_instr this_instr) {
    return (this_instr.op < BEQ);
}

int issueALU(struct input_instr this_instr,
             struct RS_ RS,
             struct ROB_ ROB,
             struct RAT_line *RAT,
             int *int_RF,
             float *float_RF,
             int cycles) {
    //TODO
    int seatRS = hasSeatRS(this_instr, RS);

    //can't issue
    if (seatRS == -1 || !hasSeatROB(ROB))
        return FALSE;

    // update timing table
    startISSUEtable(table_index, cycles);
    RS.entity[seatRS].ttable_index = table_index;
    TimeTable[table_index].index = table_index;
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
        RS.entity[seatRS].val_2 = this_instr.rt;
        RS.entity[seatRS].tag_2 = NULL;
    } else {
        if (RAT[this_instr.rt].tag == 0) {
            if (isFloatInstr(this_instr.op)) {
                RS.entity[seatRS].val_2 = float_RF[this_instr.rt-ARF_SIZE];

            } else {
                RS.entity[seatRS].val_2 = int_RF[this_instr.rt];
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
    return TRUE;
}



int startExecALU(struct RS_line *this_RS,
                 struct ALU_ ALU,
                 int cycles) {
    //TODO
    int seatALU = hasSeatALU(this_RS, ALU);
    if (this_RS->busy == TRUE &&
        dataReadyRS(this_RS) &&
        issueComplete(this_RS, cycles) &&
        seatALU != -1 &&
        this_RS->stage == ISSUE) {

        //update timing table
        startEXECtable(this_RS->ttable_index, cycles);

        //update RS
        this_RS->stage = EXEC;
        this_RS->cycles = cycles;
        this_RS->alu_index = seatALU;

        //update ALU
        ALU.entity[seatALU].busy = TRUE;

        return TRUE;
    }

    return FALSE;
}
int exeCompleteALU(struct RS_line *this_RS, struct ALU_ ALU, int cycles) {
    int alu_index = this_RS->alu_index;
    if (cycles - this_RS->cycles >= ALU.entity[alu_index].exec_cycle &&
        this_RS->stage == EXEC) {
        //TODO
        return TRUE;
    }
    return FALSE;
}

int startWback(struct RS_line *this_RS,
        struct RS_ RS,
        struct ALU_ ALU,
        struct ROB_ ROB,
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
             RS.entity[i].val_2 = result;
             RS.entity[i].tag_2 = NULL;
        }
    }
    return TRUE;
}

int readyCommitROB(struct ROB_line this_ROB) {
    if (this_ROB.finished == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

int startCommit(struct ROB_line *this_ROB,
    struct RAT_line *RAT,
             int *int_RF,
             float *float_RF,
             int cycles) {
     //TODO
    // update timing table
    startCOMMITtable(this_ROB->ttable_index, cycles);
    //update ROB
    this_ROB->busy = FALSE;
    this_ROB->finished = FALSE;
    // update RAT
    RAT[this_ROB->dst].tag = 0;
     // int RF
    if (this_ROB->dst < ARF_SIZE) {
        int result = (int) this_ROB->val;
        // update int_RF
        int_RF[this_ROB->dst] = result;
    } else if (this_ROB->dst >= ARF_SIZE) {
        float result = (float) this_ROB->val;
        // update float_RF
        float_RF[this_ROB->dst - ARF_SIZE] = result;
    }
    return TRUE;
}

int notFinishROB(struct ROB_line *this_ROB) {
    if (this_ROB->busy == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}
