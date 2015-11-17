#ifndef UTIL_H_
#define UTIL_H_

/*
 * Some Configurations.
 */
#define MEM_SIZE 1000
#define ARF_SIZE 20
#define MAX_ALU 16 // supported maximum kind of alu

// ALU type
#define ALU_ADDI 1 // integer add/sub
#define ALU_MULI 2 // integer multiply
#define ALU_DIVI 3 // integer divide
#define ALU_ADDD 4 // float add/sub
#define ALU_MULD 5 // float multiply
#define ALU_DIVD 6 // float divide
#define LOAD_STORE 7 // load/store unit


/*
 * This head file declares some global data structures and configurations.
 */

/*
 * Register Station line.
 */
struct RS_line {
     //TODO
    int instr_type; // same as ALU_line
    int busy;
    int instr_addr; // which instr is loaded.
    struct ROB_line *dst;
    struct ROB_line *tag_1;
    struct ROB_line *tag_2;
    float val_1;
    float val_2;
    int stage; // 1: exec; 2: write; 3:commit
    int cycles; // how many cycles in this stage;
};

/*
 * struct @ROB_line store all information in ONE line of ROB
 * */
struct ROB_line {
    int dst; // index of ARF (start from 1)
    float val; // value of result
    int finished; // finished or not
};

/*
 * struct @instr stores all information in ONE instruction
 * use index of integer Register & float Register to describe
 * used in instruction buffer
 */
// for all operand, float index += ARF_SIZE
struct input_instr {
    int op; // instruction name. more details in instr.h
    float rs; // source register
    float rt; // target register or immediate
    float rd; // destination register
    int valid; // because instuction buffer is constructed by this struct, thus
               // I need a valid flag to whether there is a instr in buffer.
};

// Register alias table
struct RAT_line {
    int tag; // 0: integer register file; 1: float register file; 2: ROB
    struct ROB_line *re_name; // reference to ROB if tag = 2
};

// ALU function unit
struct ALU_line {
    int type; // which type ALU it is 1: int add/sub; 2:int mul; 3:int div
              // 4: float add/sub; 5:float mul; 6: float div
    int exec_cycle; // how much cycles exec takes
    int mem_cycle; // how much cycles mem takes
    int busy; // this ALU is busy or not

};

#endif
