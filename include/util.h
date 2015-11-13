#ifndef UTIL_H_
#define UTIL_H_

/*
 * This head file declares some global data structures and configurations.
 */

/*
 * struct @buffer_line store all information in ONE line of ROB
 * */
struct buffer_line {
    int dst; // index of ARF (start from 1)
    float val; // value of result
    int finished; // finished or not
};

/*
 * struct @instr stores all information in ONE instruction
 * For simplicity, use pointer to ROB
 */
struct instr {
    int op; // instruction name. more details in instr.h
    struct buffer_line *dst_tag; // point to destination value.
    struct buffer_line *tag_1;
    struct buffer_line *tag_2;
    float val_1;
    float val_2;
    int status; // TODO: use status to declare which step it is in I/E/W/C
    int cycles; // TODO: how many cycles it is in EXEC steps
};

#endif
