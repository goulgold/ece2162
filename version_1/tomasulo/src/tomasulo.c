/*
 ============================================================================
 Name        : tomasulo.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <locale.h>
#include <ctype.h>
#include <limits.h>

#include "config.h"

typedef struct {
	uint32_t start, size;
	uint8_t *mem;
} mem_region_t;

mem_region_t MEM_REGIONS[] = { { MEM_INST_START, MEM_INST_SIZE, NULL }, {
MEM_DATA_START, MEM_DATA_SIZE, NULL } };

#define MEM_NREGIONS (sizeof(MEM_REGIONS)/sizeof(mem_region_t))

int32_t ARF[NUM_REGISTERS] = { 0 };
uint32_t RAT[NUM_REGISTERS] = { 0 };

float ARF_FP[NUM_REGISTERS] = { 0.0 };
float RAT_FP[NUM_REGISTERS] = { 0.0 };

int RUN_BIT = TRUE;
int ROB_size = 32;             //initialization of ROB size, in case not defined
uint32_t stat_cycles = 0;

/***************************************************************/
/*                                                             */
/* Procedure: convert float into storage available format      */
/*                                                             */
/***************************************************************/
uint32_t float_to_int(float input) {
	char buffer[120] = { '\0' };
	char *ret;
	uint32_t flag, count = 0;
	if (input < 0) {
		flag = 1;
	}
	sprintf(buffer, "%f", input);
	if (strchr(buffer, '.')) {
		ret = strchr(buffer, '.');
		count = strlen(ret) - 1;
		if (input < 0) {
			flag = 1;
			input = -input;
		}
		for (int i = 0; i < count; i++) {
			printf("%f\n", input);
			input *= 10;
		}
	}

	uint32_t part_num = (uint32_t) input;
	uint32_t part_sig = (flag << 31) | (count << 24);
	return part_sig | part_num;
}
/***************************************************************/
/*                                                             */
/* Procedure: retrieve float from storage device		       */
/*                                                             */
/***************************************************************/
float int_to_float(uint32_t input) {
	uint32_t flag = (input >> 31);
	printf("flag = %u\n", flag);
	uint32_t count = (input >> 24) & 0x7F;
	printf("count = %u\n", count);
	uint32_t num = (input >> 0) & 0xFFFFFF;
	printf("num = %u\n", num);
	float result = (float) num;
	for (int i = 0; i < count; i++) {
		result /= 10;
	}
	if (flag) {
		result = -result;
	}
	return result;
}
/***************************************************************/
/*                                                             */
/* Procedure: mem_read_32                                      */
/*                                                             */
/***************************************************************/
uint32_t mem_read_32(uint32_t address) {
	int i;
	for (i = 0; i < MEM_NREGIONS; i++) {
		if (address >= MEM_REGIONS[i].start
				&& address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
			uint32_t offset = address - MEM_REGIONS[i].start;

			return (MEM_REGIONS[i].mem[offset + 3] << 24)
					| (MEM_REGIONS[i].mem[offset + 2] << 16)
					| (MEM_REGIONS[i].mem[offset + 1] << 8)
					| (MEM_REGIONS[i].mem[offset + 0] << 0);
		}
	}

	return 0;
}
/***************************************************************/
/*                                                             */
/* Procedure: mem_write_32                                     */
/*                                                             */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value) {
	int i;
	for (i = 0; i < MEM_NREGIONS; i++) {
		if (address >= MEM_REGIONS[i].start
				&& address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
			uint32_t offset = address - MEM_REGIONS[i].start;

			MEM_REGIONS[i].mem[offset + 3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset + 2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset + 1] = (value >> 8) & 0xFF;
			MEM_REGIONS[i].mem[offset + 0] = (value >> 0) & 0xFF;
			return;
		}
	}
}
/***************************************************************/
/*                                                             */
/* Procedure : get integer between offset and end char         */
/*                                                             */
/***************************************************************/
int getint(int offset, int end, char* line_buffer) {
	int isNew = TRUE;
	int nWordPosBegin = offset;
	int nWordPosEnd = end;
	for (int i = offset; i != end; i++) {
		if (isdigit(line_buffer[i]) || line_buffer[i] == '-') {
			if (isNew) {
				nWordPosBegin = i;
				isNew = FALSE;
			}
		} else {
			if (!isNew) {
				nWordPosEnd = i;
			}
		}
	}
	if (!isNew) {
		char word[MAX_LEN_OF_ONE_LINE] = { 0 };
		strncpy(word, line_buffer + nWordPosBegin, nWordPosEnd - nWordPosBegin);
		return atoi(word);
	} else {
		return -1;
	}
}
/***************************************************************/
/*                                                             */
/* Procedure : get float between offset and end char      	   */
/*                                                             */
/***************************************************************/
float getfp(int offset, int end, char* line_buffer) {
	int isNew = TRUE;
	int nWordPosBegin = offset;
	int nWordPosEnd = end;
	for (int i = offset; i != end; i++) {
		if (isdigit(line_buffer[i]) || line_buffer[i] == '-') {
			if (isNew) {
				nWordPosBegin = i;
				isNew = FALSE;
			}
		} else {
			if (!isNew) {
				nWordPosEnd = i;
			}
		}
	}
	if (!isNew) {
		char word[MAX_LEN_OF_ONE_LINE] = { 0 };
		strncpy(word, line_buffer + nWordPosBegin, nWordPosEnd - nWordPosBegin);
		return (float) atof(word);
	} else {
		return -1.0;
	}
}
/***************************************************************/
/*                                                             */
/* Procedure : encode                                          */
/*                                                             */
/***************************************************************/
uint32_t encode(char* line_buffer) {
	int rd, rs, rt, im, offset = -1;
	int op, count = 0;
	uint32_t result = 0;
	if (strstr(line_buffer, ".d")) {
		char *ret = line_buffer;
		while (strstr(ret, " ")) {
			int begin = strstr(ret, " ") - ret;
			int end;
			if (strstr(ret, ",")) {
				end = strstr(ret, ",") - ret;
			} else {
				end = strlen(ret);
			}
			switch (count) {
			case 0:
				rd = getint(begin, end, ret) << 11;
				break;
			case 1:
				rs = getint(begin, end, ret) << 21;
				break;
			case 2:
				rt = getint(begin, end, ret) << 16;
				break;
			}
			count++;
			ret += end + 1;
		}
		if (strstr(line_buffer, "add")) {
			op = DADD;
		} else if (strstr(line_buffer, "sub")) {
			op = DSUB;
		} else {
			op = DMULT;
		}
		result = rs | rt | rd | op;
	} else if (strstr(line_buffer, "sd ") || strstr(line_buffer, "ld ")) {
		char *ret = line_buffer;
		while (strstr(ret, " ") || strstr(ret, "(")) {
			int begin;
			int end;
			if (strstr(ret, " ")) {
				begin = strstr(ret, " ") - ret;
				if (strstr(ret, ",")) {
					end = strstr(ret, ",") - ret;
					rt = getint(begin, end, ret) << 16;
				} else {
					end = strstr(ret, "(") - ret;
					offset = getint(begin, end, ret);
				}
			} else {
				begin = strstr(ret, "(") - ret;
				end = strstr(ret, ")") - ret;
				rs = getint(begin, end, ret) << 21;
			}
			count++;
			ret += end + 1;
		}
		if (strstr(line_buffer, "sd")) {
			op = SD << 26;
		} else {
			op = LD << 26;
		}
		if (offset < 0) {
			offset += USHRT_MAX + 1;
		}
		result = op | rs | rt | offset;
	} else if (strstr(line_buffer, "beq ") || strstr(line_buffer, "bne ")) {
		char *ret = line_buffer;
		while (strstr(ret, " ")) {
			int begin = strstr(ret, " ") - ret;
			int end;
			if (strstr(ret, ",")) {
				end = strstr(ret, ",") - ret;
			} else {
				end = strlen(ret);
			}
			switch (count) {
			case 0:
				rs = getint(begin, end, ret) << 21;
				break;
			case 1:
				rt = getint(begin, end, ret) << 16;
				break;
			case 2:
				offset = getint(begin, end, ret);
				break;
			}
			count++;
			ret += end + 1;
		}
		if (strstr(line_buffer, "bne")) {
			op = BNE << 26;
		} else {
			op = BEQ << 26;
		}
		if (offset < 0) {
			offset += USHRT_MAX + 1;
		}
		result = op | rs | rt | offset;
	} else if (strstr(line_buffer, "addi")) {
		char *ret = line_buffer;
		while (strstr(ret, " ")) {
			int begin = strstr(ret, " ") - ret;
			int end;
			if (strstr(ret, ",")) {
				end = strstr(ret, ",") - ret;
			} else {
				end = strlen(ret);
			}
			switch (count) {
			case 0:
				rs = getint(begin, end, ret) << 21;
				break;
			case 1:
				rt = getint(begin, end, ret) << 16;
				break;
			case 2:
				im = getint(begin, end, ret);
				break;
			}
			count++;
			ret += end + 1;
		}
		int op = ADDI << 26;
		if (im < 0) {
			im += USHRT_MAX + 1;
		}
		result = op | rs | rt | im;
	} else if (strstr(line_buffer, "add") || strstr(line_buffer, "sub")) {
		char *ret = line_buffer;
		while (strstr(ret, " ")) {
			int begin = strstr(ret, " ") - ret;
			int end;
			if (strstr(ret, ",")) {
				end = strstr(ret, ",") - ret;
			} else {
				end = strlen(ret);
			}
			switch (count) {
			case 0:
				rd = getint(begin, end, ret) << 11;
				break;
			case 1:
				rs = getint(begin, end, ret) << 21;
				break;
			case 2:
				rt = getint(begin, end, ret) << 16;
				break;
			}
			count++;
			ret += end + 1;
		}
		if (strstr(line_buffer, "add")) {
			op = ADD;
		} else {
			op = SUB;
		}
		result = rs | rt | rd | op;
	}
	return result;
}
/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/***************************************************************/
void cycle() {
//	pipe_cycle();                 //TO-DO

	stat_cycles++;                 //TO-DO
}
/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/***************************************************************/
void go() {
	if (RUN_BIT == FALSE) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating...\n\n");
	while (RUN_BIT)
		cycle();
	printf("Simulator halted\n\n");
}
/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/***************************************************************/
void init_memory() {
	int i;
	for (i = 0; i < MEM_NREGIONS; i++) {
		MEM_REGIONS[i].mem = malloc(MEM_REGIONS[i].size);
		memset(MEM_REGIONS[i].mem, 0, MEM_REGIONS[i].size);
	}
}
/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/************************************************************/
void initialize(char *program_filename) {

	init_memory();
//	pipe_init();              //TO-DO

	FILE * prog;
	int ii, instruction;

	/* Open program file. */
	prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	/* Read in the program. */
	ii = 0;
	int line_number = 0;
	char line_buffer[MAX_LEN_OF_ONE_LINE];
	while (fgets(line_buffer, sizeof(line_buffer), prog) != NULL) {
		int len = strlen(line_buffer);
		int end = len;
		int begin = 0;
		int middle, index, int_val = 0;
		float fp_val = 0.0;
		char *ret = line_buffer;
		switch (line_number) {
		case 0:
			if (strstr(ret, "ROB entries = ")) {
				ROB_size = getint(begin, end, ret);
			}
			break;
		case 1:
			while (strchr(ret, '=')) {
				middle = strchr(ret, '=') - ret;
				if (strchr(ret, ',')) {
					end = strchr(ret, ',') - ret;
				} else {
					end = strlen(ret);
				}
				index = getint(begin, middle, ret);
				if (strchr(ret, 'R')) {
					begin = strchr(ret, 'R') - ret;
					int_val = getint(middle, end, ret);
					ARF[index] = int_val;
				} else if (strchr(ret, 'F')) {
					begin = strchr(ret, 'F') - ret;
					fp_val = getfp(middle, end, ret);
					ARF_FP[index] = fp_val;
				}
				ret += end + 1;
			}
			break;
		case 2:
			while (strchr(ret, '=')) {
				middle = strchr(ret, '=') - ret;
				if (strchr(ret, ',')) {
					end = strchr(ret, ',') - ret;
				} else {
					end = strlen(ret);
				}
				index = getint(begin, middle, ret);
				if (strchr(ret, 'M')) {
					begin = strchr(ret, 'M') - ret;
					fp_val = getfp(middle, end, ret);
					//TO-DO
					mem_write_32(MEM_DATA_START + 4 * index, fp_val);
				}
				ret += end + 1;
			}
			break;
		default:
			if (isalpha(line_buffer[0])) {
				instruction = encode(line_buffer);
				mem_write_32(MEM_INST_START + ii, instruction);
				ii += 4;
			}
			break;
		}
		line_number++;
	}
	printf("Read %d words from program into memory.\n\n", ii / 4);
	fclose(prog);
}
/************************************************************/
/*                                                          */
/* Procedure : main		                                    */
/*                                                          */
/************************************************************/
int main(int argc, char *argv[]) {
	// 1.test arguments is valid or not
	if (argc != 2) {
		printf("Usage: %s <input file>\n", argv[0]);
		exit(1);
	}
	// 2.initialize all parameters and allocate memory
	initialize(argv[1]);
	// 3.enter pipe line cycle
	RUN_BIT = TRUE;
	go();
	return 0;
}
