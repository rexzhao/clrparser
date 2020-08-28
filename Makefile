OBJ=$(patsubst %.c, %.o, $(wildcard *.c))

BIN=cclr

all : ${BIN}

${BIN} : ${OBJ}
	cc -g -o $@ $^

%.o : %.c
	cc -g -c -o $@ $<

clean : 
	rm -vf ${BIN} ${OBJ}

run :
	./${BIN} /mnt/f/cli/cli/bin/Debug/cli.exe

