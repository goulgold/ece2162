#ifndef INSTR_H_
#define INSTR_H_
/* All instructions are described by 5-digit unsigned integer
 * Different kind of Instructions has Different prefix 1-digit.
 * 1#### : arithmetic/logical instructions
 * 2#### : control flow instructions
 * 3#### : load/store Instructions
 */

/* ****
 * 1#### : arithmetic/logical instructions
 * ****/

#define ADD 10001 // Add Rd, Rs, Rt | Rd = Rs + Rt | integer add
#define ADD_D 10002 // Add_d Fd, Fs, Ft | Fd = Fs + Ft | float add
#define ADDI 10003 // Addi Rd, Rs, immediate | Rd = Rs + immediate | immediate add

#define SUB 10004 // Sub Rd, Rs, Rt | Rd = Rs - Rt |  integer sub
#define SUB_D 10005 // Sub_d Fd, Fs, Ft | Fd = Fs - Ft | float sub
#define SUBI 10006 // Subi Fd, Fs, immediate | Rd = Rs - immediate | immediate sub

#define MULT 10007 // Mult Rd, Rs, Rt | Rd = Rs * Rt | integer multiply
#define MULT_D 10008 // Mult_d Fd, Fs, Ft | Fd = Fs * Ft | float multiply
#define DIV 10009 // Divide Rd, Rs, Rt | Rd = Rs / Rt | integer divide
#define DIV_D 10010 // Divide.d Fd, Fs, Ft | Fd = Fs / Ft | float divide

/* ****
 * 2#### : control flow instructions
 * ****/

#define BEQ 20001 // Beq Rs, Rt, offset | If Rs==Rt then branch to PC+4+offset<<2
#define BNE 20002 // Bne Rs, Rt, offset | If Rs!=Rt then branch to PC+4+offset<<2

/* ****
 * 3#### : load/store instructions
 */

#define LD 30001 // Ld Fa, offset(Ra) | Load a single precision floating point value to Fa
#define L 30002 // L Rb, offset(Ra) | Load a integer value to Rb
#define SD 30003 // Sd Fa, offset(Ra) | Store a single precision floating point value to memory
#define S 30004 // S Rb, offse(Ra) | Store a integer value to memory

#endif
