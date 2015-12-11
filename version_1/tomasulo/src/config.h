#ifndef CONFIG_H_
#define CONFIG_H_

#define TRUE  1
#define FALSE 0
#define NUM_REGISTERS 32
#define MAX_LEN_OF_ONE_LINE 120
/***************************************************************/
/*                                                             */
/* Definition: isa definition (mips)                           */
/*                                                             */
/***************************************************************/
#define DADD	44	//101100
#define DSUB	46	//101110
#define DMULT	28	//011100

#define SD 		63 	//111111
#define LD 		55	//110111

#define BNE 	5	//000101
#define BEQ		4	//000100

#define ADDI	8	//001000

#define ADD 	32 	//100000
#define SUB 	34 	//100010
#define SPECIAL 0	//000000
/***************************************************************/
/*                                                             */
/* Definition: memory allocation                               */
/*                                                             */
/***************************************************************/
#define MEM_DATA_START  0x10000000
#define MEM_DATA_SIZE   0x00000100
#define MEM_INST_START  0x00400000
#define MEM_INST_SIZE   0x00000100
/***************************************************************/
/*                                                             */
/* Definition: unify settings for all instructions	           */
/*                                                             */
/***************************************************************/
#define ISSUE_CYCLE 1
#define WB_CYCLE 1
#define COMMIT_CYCLE 1
/***************************************************************/
/*                                                             */
/* Definition: integer adder specified settings                */
/* Include: addi, add, sub, load & store address calculation   */
/*                                                             */
/***************************************************************/
#define NUM_RS_ADD 2		//reservation station entry number
#define EX_CYCLE_ADD 1 		//cycles in ex
#define MEM_CYCLE_ADD 1		//cycles in mem
#define ALU_ADD 1			//alu
/***************************************************************/
/*                                                             */
/* Definition: fp adder specified settings	                   */
/* Include: add.d, sub.d                                       */
/*                                                             */
/***************************************************************/
#define NUM_RS_DADD 3
#define EX_CYCLE_DADD 3
#define ALU_DADD 1
/***************************************************************/
/*                                                             */
/* Definition: fp multiplier specified settings	               */
/* Include: mult.d                                             */
/*                                                             */
/***************************************************************/
#define NUM_RS_DMULT 2
#define EX_CYCLE_DMULT 20
#define ALU_DMULT 1
/***************************************************************/
/*                                                             */
/* Definition: load & store specified settings	               */
/*                                                             */
/***************************************************************/
#define NUM_RS_LD 3
#define EX_CYCLE_LD 1
#define MEM_CYCLE_LD 4
#define ALU_LD 1

#endif
