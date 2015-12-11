INCLUDE_DIR = ./include
PROGRAM = simulator
CC = gcc
SOURCE_FILES = $(wildcard *.c)
HEAD_FILES = $(wildcard ${INCLUDE_DIR}/*.h)
OBJ_FILES = $(patsubst %.c,%.o,${SOURCE_FILES})
CC_FLAGS = -g -std=c99 -Wall -O0

all: ${PROGRAM}

${PROGRAM} : ${OBJ_FILES}
	${CC} -o ${PROGRAM} ${OBJ_FILES}

${OBJ_FILES} : ${HEAD_FILES}

${OBJ_FILES} : %.o: %.c
	${CC} -c $< ${CC_FLAGS} -I${INCLUDE_DIR}

.PHONY : clean

clean:
	rm -f *.o ${PROGRAM}


