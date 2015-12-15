#include "print.h"
#include <stdio.h>
#include "util.h"

extern struct RS_ RS; // register station
extern struct ALU_ ALU;
extern struct ROB_ ROB;
extern struct LsQueue LSQ;
extern struct RAT_line *RAT;
extern struct input_instr *instr_mem;
extern float *data_mem;
extern int *int_RF;
extern float *float_RF;

int printStatus(int cycles) {
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
    printf("%f\n",data_mem[i]);
    }
    */

    //print Reservation Station
    printf ("Reservation Station:\n");
    printf ("type\tbusy\tdst\ttag1\ttag2\tval1\tval2\tcycles\tstage\n");
    for (int i = 0; i < RS.size; ++i) {
        printf("%d\t%d\t", RS.entity[i].instr_type, RS.entity[i].busy);
        printPointROB(RS.entity[i].dst);
        printPointROB(RS.entity[i].tag_1);
        printPointROB(RS.entity[i].tag_2);
        printf("%f\t", RS.entity[i].val_1);
        printf("%f\t", RS.entity[i].val_2);
        printf("%d\t", RS.entity[i].cycles);
        printf("%d\t", RS.entity[i].stage);
        printf("\n");
    }

    // print Load Store Queue
    printf("Load/Store Queue:\n");
    printf ("type\tbusy\tmem_addr\tmemval\tdst\tbase\toffset\tcycles\tstage\n");
    for (int i = 0; i < LSQ.size; ++i) {
        printf("%d\t", LSQ.entity[i].alu_type);
        printf("%d\t", LSQ.entity[i].busy);
        printf("%d\t", LSQ.entity[i].exec_cycle);
        printf("%d\t", LSQ.entity[i].mem_cycle);
        printf("\n");
    }

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

    //Re-order Buffer (ROB)
    printf ("ROB:\n");
    printf ("index\tbusy\tdes\tval\tfinished\n");
    for (int i = 0; i < ROB.size; ++i) {
        printf("%d\t", ROB.entity[i].index);
            printf("%d\t%d\t%f\t%d\n",
                    ROB.entity[i].busy,
                    ROB.entity[i].dst,
                    ROB.entity[i].val,
                    ROB.entity[i].finished);
            printf("\n");
    }
    return 0;
}

void printPointROB(struct ROB_line *this_ROB) {
    if (this_ROB == NULL) {
        printf("null\t");
    } else {
        printf("%d\t", this_ROB->index);
    }
}
