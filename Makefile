CC=gcc
FLAGS=-Wall -Wextra -Wshadow -std=c99 -g -O3
LIBS=-lpng -lm
BIN=/usr/bin/
OBJS=

all: ./src/main.c ${OBJS}
	${CC} ./src/main.c ${OBJS} ${FLAGS} ${LIBS} -o ./bin/ifyt

clean:
	rm ./bin/*
