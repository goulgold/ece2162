/*
 ============================================================================
 Name        : test.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <limits.h>
#include <assert.h>

#include "config.h"
#include "upper.h"

#define TRUE  1
#define FALSE 0
#define RUN_BIT 1
#define MAX_LEN_OF_ONE_LINE 120

int stat_inst_fetch, stat_inst_retire = 0;

int32_t ARF[NUM_REGISTERS] = { 0 };

float ARF_FP[NUM_REGISTERS] = { 0.0 };

/*Global Branch Prediction Vaiables*/
uint8_t ghr = 0;
uint8_t pht[256];
buffer_t btb[1024];

uint32_t start, end = 0;
uint32_t b = end + 1;
uint32_t r = RS_SIZE;
uint32_t h, d = 0;
int CDB = 1;
float buffer;

int alu_add, alu_dadd, alu_dmult, alu_ld = 0;

Pipe_State pipe;

int RS_avalible(uint32_t opcode, uint32_t funct) {
	if (opcode == SPECIAL) {
		switch (funct) {
		case ADD:
		case SUB:
			if (RSS.num_add != NUM_RS_ADD)
				return 1;
			break;
		case DADD:
		case DSUB:
			if (RSS.num_dadd != NUM_RS_DADD)
				return 2;
			break;
		case DMULT:
			if (RSS.num_dmult != NUM_RS_DMULT)
				return 3;
			break;
		}
	} else if (opcode == ADDI) {
		if (RSS.num_add != NUM_RS_ADD)
			return 1;
	} else if (opcode == LD || opcode == SD) {
		if (RSS.num_ld != NUM_RS_LD)
			return 4;
	} else if (opcode == BEQ || opcode == BNE) {
		return 5;
	}
	return 0;
}

uint32_t prediction(uint32_t branch_pc) {

	if ((btb[BTB_IDX(branch_pc)].tag != branch_pc)
			|| (btb[BTB_IDX(branch_pc)].valid == 0)) {
		pipe.predicted_dest = branch_pc + 4;
		return 0;
	} else if ((btb[BTB_IDX(branch_pc)].tag == branch_pc)
			&& (btb[BTB_IDX(branch_pc)].valid == 1)) {
		if ((btb[BTB_IDX(branch_pc)].U == 1)
				|| (pht[GSHARE(ghr, branch_pc)] > 1)) {
			pipe.predicted_dest = btb[BTB_IDX(branch_pc)].target;
			return 1;
		}
	}

	pipe.predicted_dest = branch_pc + 4;
	return 0;
}

void pipe_recover(int flush, uint32_t dest) {
	/* if there is already a recovery scheduled, it must have come from a later
	 * stage (which executes older instructions), hence that recovery overrides
	 * our recovery. Simply return in this case. */
	if (pipe.branch_recover)
		return;

	/* schedule the recovery. This will be done once all pipeline stages simulate the current cycle. */
	pipe.branch_recover = 1;
	pipe.branch_flush = flush;
	pipe.branch_dest = dest;
}
/***************************************************************/
/*                                                             */
/* Stage: Commit									           */
/*                                                             */
/***************************************************************/
void stage_commit() {
	h = start;
	if (ROB[h].Ready == 1) {
		/* Instruction is at the head of the ROB(entry h) and ROB[h] == 1 */
		d = ROB[h].Dest; /* register dest, if exists*/
		switch (ROB[h].Instruction) {
		case BNE:
		case BEQ:
			if ((ROB[h].predicted_dir != ROB[h].branch_taken)
					|| ((btb[BTB_IDX(ROB[h].pc)].valid == 0)
							|| (btb[BTB_IDX(ROB[h].pc)].tag != ROB[h].pc))) {
				/*clear ROB[h], RegisterStat; fetch branch dest;*/
				if (ROB[h].branch_taken == 1)
					pipe_recover(3, ROB[h].branch_dest);
				else
					pipe_recover(3, ROB[h].pc + 4);
			}
			update_branch_predictions(ROB[h].pc, ROB[h].branch_taken,
					ROB[h].branch_dest);
			break;
		case SD:
			int format = getint(ROB[h].Value);
			mem_write_32(ROB[h].Address, format);
			break;
		case ADD:
		case SUB:
		case ADDI:
			ARF[d] = (int) ROB[h].Value;
			break;
		case DADD:
		case DSUB:
		case DMULT:
			ARF_FP[d] = ROB[h].Value;
			break;
		}
		ROB[h].Busy = 0;
		/* free up dest register if no one else is writing it */
		if (RegisterStat[d].Reorder == h) {
			RegisterStat[d].Busy = 0;
		}
		/* free up the ROB entry */
		memset(ROB, 0, sizeof(struct ReorderBuffer));
		start++;
	}
}
/***************************************************************/
/*                                                             */
/* Stage: Write back								           */
/*                                                             */
/***************************************************************/
void stage_wb_process(int i) {
	b = RS[i].Dest;
	RS[i].Busy = 0;
	for (int x = 0; x < RS_SIZE; x++) {
		if (RS[x].Qj == b) {
			RS[x].Vj = RS[i].Vk;
			RS[x].Qj = 0;
		}
		if (RS[x].Qk == b) {
			RS[x].Vk = RS[i].Vk;
			RS[x].Qk = 0;
		}
	}
	ROB[b].Value = RS[i].Vk;	//this include the store case
	ROB[b].Ready = 1;
	stat_inst_retire++;
}

void stage_wb() {
	CDB = 1; /*CDB free*/
	for (int i = 0; i < RS_SIZE; i++) {
		if (RS[i].Busy && CDB) {
			switch (RS[i].op) {
			case ADD:
			case SUB:
			case ADDI:
			case BNE:
			case BEQ:
				if (RS[i].exe_cycle == EX_CYCLE_ADD) {
					stage_wb_process(i);
					CDB = 0; /*occupy CDB*/
				}
				break;
			case DADD:
			case DSUB:
				if (RS[i].exe_cycle == EX_CYCLE_DADD) {
					stage_wb_process(i);
					CDB = 0; /*occupy CDB*/
				}
				break;
			case DMULT:
				if (RS[i].exe_cycle == EX_CYCLE_DMULT) {
					stage_wb_process(i);
					CDB = 0; /*occupy CDB*/
				}
				break;
			case SD:
				if (RS[i].exe_cycle == EX_CYCLE_LD) {
					stage_wb_process(i);
					CDB = 0;
				}
				break;
			case LD:
				if (RS[i].mem_cycle == MEM_CYCLE_LD) {
					ARF_FP[RS[i].Vk] = buffer;
					CDB = 0;
				}
				break;
			}
		}
	}
	return;
}
/***************************************************************/
/*                                                             */
/* Stage: Memory									           */
/*                                                             */
/***************************************************************/
void stage_mem() {
	CDB = 1;
	for (int i = 0; i < RS_SIZE; i++) {
		if (RS[i].op == LD && RS[i].exe_cycle == EX_CYCLE_LD) {
			if (RS[i].mem_cycle > 0) {
				RS[i].mem_cycle++;
			} else if (RS[i].Qj == 0 && RS[i].Qk == 0
					&& CDB/* &&all stores earilier in ROB have different address*/) { //TO-DO
				CDB = 0;
				buffer = mem_read_32(RS[i].A);
				RS[i].mem_cycle++;
			}
		}
	}
	return;
}
/***************************************************************/
/*                                                             */
/* Stage: Execute									           */
/*                                                             */
/***************************************************************/
void stage_exe() {
	for (int i = 0; i < RS_SIZE; i++) {
		if (RS[i].exe_cycle > 0) {
			RS[i].exe_cycle++;
		} else if (RS[i].Qj == 0) {
			switch (RS[i].op) {
			case LD:
				/* load */
				if (alu_ld < ALU_LD /*&&there are no stores eariler in the queue*/) { //TO-DO
					alu_ld++;
					RS[i].A = RS[i].Vj + RS[i].A;
					RS[i].exe_cycle++;
				}
				break;
			case SD:
				/* store */
				if (alu_ld < ALU_LD/*&&store at queue head*/) { //TO-DO
					alu_ld++;
					h = RS[i].Dest;
					ROB[h].Address = RS[i].Vj + RS[i].A;
					RS[i].exe_cycle++;
				}
				break;
			case DMULT:
				/* fp multiplier */
				if (RS[i].Qk == 0 && alu_dmult < ALU_DMULT) {
					alu_dmult++;
					RS[i].Vk = RS[i].Vj * RS[i].Vk;
					RS[i].exe_cycle++;
				}
				break;
			case DADD:
			case DSUB:
				/* fp adder*/
				if (RS[i].Qk == 0 && alu_dadd < ALU_DADD) {
					alu_dadd++;
					if (RS[i].op == DADD) {
						RS[i].Vk = RS[i].Vj + RS[i].Vk;
					} else {
						RS[i].Vk = RS[i].Vj - RS[i].Vk;
					}
					RS[i].exe_cycle++;
				}
				break;
			case ADDI:
				/* integer adder */
				if (RS[i].Qk == 0 && alu_add < ALU_ADD) {
					alu_add++;
					RS[i].Vk = RS[i].Vj + RS[i].A;
				}
				break;
			case ADD:
			case SUB:
				/* integer adder */
				if (RS[i].Qk == 0 && alu_add < ALU_ADD) {
					alu_add++;
					if (RS[i].op == ADD) {
						RS[i].Vk = RS[i].Vj + RS[i].Vk;
					} else {
						RS[i].Vk = RS[i].Vj - RS[i].Vk;
					}
				}
				break;
			case BEQ:
				/* integer adder */
				if (RS[i].Qk == 0 && alu_add < ALU_ADD) {
					alu_add++;
					if (RS[i].Vj == RS[i].Vk) {
						RS[i].Vk = 1;
					} else {
						RS[i].Vk = 0;
					}
				}
				break;
			case BNE:
				/* integer adder */
				if (RS[i].Qk == 0 && alu_add < ALU_ADD) {
					alu_add++;
					if (RS[i].Vj == RS[i].Vk) {
						RS[i].Vk = 0;
					} else {
						RS[i].Vk = 1;
					}
				}
				break;
			}
		}
	}
	return;
}
/***************************************************************/
/*                                                             */
/* Stage: Issue									               */
/*                                                             */
/***************************************************************/
void stage_issue() {

	if (RUN_BIT == 0) {
		pipe.PC += 4;
		return;
	}

	uint32_t branch, opcode, funct;

	/* Allocate an op and send it down the pipeline. */
	Pipe_Op *op = malloc(sizeof(Pipe_Op));
	memset(op, 0, sizeof(Pipe_Op));

	op->instruction = mem_read_32(pipe.PC);
	if (op->instruction == -1)
		return;
	op->pc = pipe.PC;

	/* set up info fields as necessary */
	uint32_t rs = (op->instruction >> 21) & 0x1F;
	uint32_t rt = (op->instruction >> 16) & 0x1F;
	uint32_t rd = (op->instruction >> 11) & 0x1F;
	uint32_t imm16 = (op->instruction >> 0) & 0xFFFF;
	uint32_t se_imm16 = imm16 | ((imm16 & 0x8000) ? 0xFFFF8000 : 0);
	uint32_t opcode = (op->instruction >> 26) & 0x3F;
	uint32_t funct = (op->instruction >> 0) & 0x3F;

	/*check the availability of Reservation Station (r) and ROB (b) */
	op->type = RS_available(opcode, funct);

	if (fabs(start, end) == 0 || op->type == 0) {
		return;
	} else {
		/*allocate reservation station entry*/
		for (int i = 0; i < RS_SIZE; i++) {
			if (!RS[i].Busy) {
				uint32_t r = (uint32_t) i;
				break;
			}
		}
		b = ++end;
		ROB[b].branch_dest = op->pc + 4 + (se_imm16 << 2);
	}

	if (RegisterStat[rs].Busy) {
		/*in-flight instr. writes rs*/
		h = RegisterStat[rs].Reorder;
		if (ROB[h].Ready) {
			/*Instr completed already*/
			RS[r].Vj = ROB[h].Value;
			RS[r].Qj = 0;
		} else {
			/*wait for instruction*/
			RS[r].Qj = h;
		}
	} else {
		if (op->type == 2 || op->type == 3) {
			RS[r].Vj = ARF_FP[rs];
		} else {
			RS[r].Vj = (float) ARF[rs];
		}
		RS[r].Qj = 0;
	}
	RS[r].Busy = 1;
	RS[r].Dest = b;
	ROB[b].Instruction = opcode | funct;
	ROB[b].Dest = rd;
	ROB[b].Ready = 0;

	if (RegisterStat[rt].Busy) {
		/*in-flight instr. writes rt*/
		h = RegisterStat[rt].Reorder;
		if (ROB[h].Ready) {
			/*Instr completed already*/
			RS[r].Vk = ROB[h].Value;
			RS[r].Qk = 0;
		} else {
			/*wait for instruction*/
			RS[r].Qk = h;
		}
	} else {
		if (op->type == 2 || op->type == 3) {
			RS[r].Vk = ARF_FP[rt];
		} else {
			RS[r].Vk = (float) ARF[rt];
		}
		RS[r].Qj = 0;
	}
	/* for floating point operations */
	if (op->type == 2 || op->type == 3) {
		RegisterStat[rd].Reorder = b;
		RegisterStat[rd].Busy = 1;
		ROB[b].Dest = rd;
	}
	/* for loads*/
	if (opcode == LD) {
		RS[r].A = se_imm16;
		RegisterStat[rt].Reorder = b;
		RegisterStat[rt].Busy = 1;
		ROB[b].Dest = rt;
	}
	/* for stores*/
	if (opcode == SD) {
		RS[r].A = se_imm16;
	}
	RS[r].exe_cycle = 0;
	stat_inst_fetch++;
	/* prediction */
	branch = (opcode == BEQ || opcode == BNE);

	if (branch == 1) {
		ROB[b].predicted_dir = prediction(pipe.PC);
		ROB[b].predicted_dest = pipe.predicted_dest;

		pipe.PC = ROB[b].predicted_dest;
		stat_inst_fetch++;
		return;
	}

	/* update PC if not predicted*/
	pipe.PC += 4;
}
