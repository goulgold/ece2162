#ifndef UTIL_H_
#define UTIL_H_

/*
 * Some Configurations.
 */
#define MEM_SIZE 1000
#define ARF_SIZE 24
#define MAX_ALU 16 // supported maximum kind of alu
#define MAX_DISPLAY 8 // in printStatus, maximum size of one line

#define FALSE 0
#define TRUE 1

// RS Stage
#define FREE 0
#define ISSUE 1
#define EXEC 2
#define WBACK 3
#define COMMIT 4


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
    int alu_type; // same as ALU_line
    int alu_index; // use which alu to exec
    int instr_type; // same as instr.h
    int busy;
    int instr_addr; // which instr is loaded.
    int mem_addr; //for load/store instr
    struct ROB_line *dst; //destination
    struct ROB_line *tag_1; // source
    struct ROB_line *tag_2; //target
    float val_1; // source
    float val_2; // target
    int stage; // 1:issue 2: exec; 3: write; 4:commit
    int cycles; // start cycles of this stage;
};

// Reservation Station
struct RS_ {
    int size;
    struct RS_line *entity;
};


/*
 * struct @ROB_line store all information in ONE line of ROB
 * */
struct ROB_line {
    int index; // index of ROB (start from 1)
    int dst; // index of ARF (start from 1)
    float val; // value of result
    int finished; // finished or not
    int busy; // occupied or not
};

struct ROB_ {
    int size;
    int nextcommit;
    int nextfree;
    struct ROB_line *entity;
};

/*
 * struct @instr stores all information in ONE instruction
 * use index of integer Register & float Register to describe
 * used in instruction buffer
 */
// for all operand, float index += ARF_SIZE
struct input_instr {
    int op; // instruction name. more details in instr.h
    int rs; // source register
    int rt; // target register or immediate
    int rd; // destination register
    int valid; // because instuction buffer is constructed by this struct, thus
               // I need a valid flag to whether there is a instr in buffer.
};

// Register alias table
struct RAT_line {
    int tag; // 0: register file; 1: ROB
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

struct ALU_ {
    int size;
    struct ALU_line *entity;
};

#endif
