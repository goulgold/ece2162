#ifndef UTIL_H_
#define UTIL_H_
#include "stdint.h"

/*
 * Some Configurations.
 */
#define MEM_SIZE 50
#define ARF_SIZE 32
#define MAX_ALU 16 // supported maximum kind of alu
#define MAX_DISPLAY 8 // in printStatus, maximum size of one line

#define FALSE 0
#define TRUE 1
#define NULL 0

// RS LSQ Stage
#define FREE 0
#define ISSUE 1
#define EXEC 2
#define MEM 3
#define WBACK 4
#define COMMIT 5


// ALU type
#define ALU_ADDI 1 // integer add/sub
#define ALU_MULI 2 // integer multiply
#define ALU_DIVI 3 // integer divide
#define ALU_ADDD 4 // float add/sub
#define ALU_MULD 5 // float multiply
#define ALU_DIVD 6 // float divide
#define LOAD_STORE 7 // load/store unit

#define BTB_IDX(X) (uint32_t) ((X) & (0x7))
#define BTB_SIZE 8



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
    struct ROB_line *dst; //destination
    struct ROB_line *tag_1; // source
    struct ROB_line *tag_2; //target
    float val_1; // source
    float val_2; // target
    int stage; // 1:issue 2: exec; 3: write; 4:commit
    int cycles; // start cycles of this stage;
    int ttable_index; // timing table index

    uint32_t pc;
    /* branch information */
    int offset;
    int is_branch;
    uint32_t branch_dest;
    int branch_taken;

    uint32_t predicted_dir;
    uint32_t predicted_dest;

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
    int ttable_index; // timing table index
    int dst; // index of ARF (start from 1)
    float val; // value of result
    int finished; // finished or not
    int busy; // occupied or not
    int store_instr; // is a store instruction or not;
    int addr; // for store instruction, where data goes.
    struct LsQueue_line* store_q; // for store instruction, which LSQ_line belongs to.
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
    char instr_line[1024];
    uint32_t pc;
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
    struct ROB_line *re_name; // reference to ROB if tag = 1
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

// Load / Store Queue like Reservation Station
struct LsQueue_line {
    int busy;
    int finished;
    int exec_cycle;
    int mem_cycle;
    int instr_type;
    int alu_type;
    int mem_addr;
    float mem_val;
    int data_ready; // the store val is ready or not
    struct ROB_line *buffer; // for store instr, where data store in ROB temporarily
    struct ROB_line *dst; // for load instr, which ROB line data goes, for store instr, where data comes from
    struct ROB_line *tag_1; //base addr
    int val_1; // base addr
    int offset; // offset addr
    int stage;
    int cycle; // which cycle this stage begins
    int ttable_index;
};

struct LsQueue {
    struct LsQueue_line *entity;
    int head;
    int tail;
    int size;
};

struct pipe_state
{
    int branch_index;
    int branch_unresovled;
    int branch_recover;
    int branch_dest;
};

typedef struct buffer_t{
        uint32_t valid,tag,target;

}buffer_t;



/* ***************************
 * Some util functions
 * **************************/

int notFinishROB(struct ROB_line *this_ROB);

// is a Float instruction, return true
int isFloatInstr(int op);

// is a sub instruction, return true
int isSubInstr(int op);

// given a instruction, get ALU type
int instr2ALUtype(struct input_instr this_instr);

// reset RS
int resetRS(struct RS_line * this_RS);

// reset a LSQ_line
int resetLSQ_line (struct LsQueue_line *this_LSQ);

int resetLSQ ();

int resetROB_line (struct ROB_line * this_ROB);

#endif
