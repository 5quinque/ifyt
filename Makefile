CC=gcc
FLAGS=-Wall -Wextra -Wshadow -std=c99 -g -O3
LIBS=-lpng -lm
BIN=/usr/bin/
OBJS=

all: ./src/ifyt.c ${OBJS}
	${CC} ./src/ifyt.c ${OBJS} ${FLAGS} ${LIBS} -o ./bin/ifyt

install:
	cp ./bin/ifyt /usr/local/bin/ifyt

uninstall:
	rm -f /usr/local/bin/ifyt

clean:
	rm ./bin/*
