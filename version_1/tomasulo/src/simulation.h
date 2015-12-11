#ifndef SIMULATION_H_
#define SIMULATION_H_

#include <stdio.h>
#include "config.h"

#define NUM_REGISTERS 32
#define ROB_SIZE 128
#define RAT_SIZE 2 * NUM_REGISTERS
#define RS_SIZE NUM_RS_ADD + NUM_RS_DADD + NUM_RS_DMULT + NUM_RS_LD

struct RAT {
	int Busy;
	uint32_t Reorder;
};

struct ReorderBuffer {
	uint32_t Instruction;
	uint32_t Dest;
	uint32_t Address;
	float Value;
	int Busy;
	int Ready;		//indicate whether the instruction is completed

	/*branch prediction*/
	uint32_t predicted_dir;
	uint32_t predicted_dest;
	uint32_t branch_taken;
	uint32_t branch_dest;
	uint32_t pc;
};

struct ReservationStation {
	int op;
	int Busy;
	int exe_cycle, mem_cycle;
	float Vj;
	float Vk;
	uint32_t Qj;
	uint32_t Qk;
	uint32_t A;
	uint32_t Dest;	//ROB index
};

struct ReservationStationSlot {
	int num_add;
	int num_dadd;
	int num_dmult;
	int num_ld;
};

struct LoadStoreQueue {
	int type;
	uint32_t index;
	uint32_t PC;
	uint32_t Addr;
	float value;
};

struct RAT RegisterStat[RAT_SIZE];
struct ReorderBuffer ROB[ROB_SIZE];
struct ReservationStation RS[RS_SIZE];
struct ReservationStationSlot RSS;

typedef struct Pipe_Op {
	/* PC of this instruction */
	uint32_t pc;
	/* raw instruction */
	uint32_t instruction;
	/* alu type */
	int type;
	/* decoded opcode and subopcode fields */
	int opcode, subop;

	uint32_t predicted_dest;

} Pipe_Op;

typedef struct Pipe_State {

	/* program counter in fetch stage */
	uint32_t PC;

	/* information for PC update (branch recovery). Branches should use this
	 * mechanism to redirect the fetch stage, and flush the ops that came after
	 * the branch as necessary. */
	int branch_recover; /* set to '1' to load a new PC */
	uint32_t branch_dest; /* next fetch will be from this PC */
	int branch_flush; /* how many stages to flush during recover? (1 = fetch, 2 = fetch/decode, ...) */

	/* multiplier stall info */
	int execution_stall, memory_stall; /* number of remaining cycles until HI/LO are ready */

	/* place other information here as necessary */

	int stall_inst, stall_data;
	int mem_request;
	int comLineBusy;
	int addrLineBusy;
	int dataLineBusy;
	int DRAMbusy[8];
	uint8_t cDone1, cDone2, aDone, dDone1, dDone2, dDone3, datDone;

	int curr_mshr;
	uint8_t mem_ibusy, insertingI, mem_dbusy, insertingD;
	uint8_t Dstayout, Istayout;
	uint32_t predicted_dest;

} Pipe_State;

/* global variable -- pipeline state */
extern Pipe_State pipe;

typedef struct buffer_t {
	uint32_t valid, tag, U, target;
} buffer_t;

#endif
