OBJ=$(patsubst %.c, %.o, $(wildcard *.c))

BIN=cclr

all : bin/${BIN}

bin/${BIN} : ${OBJ}
	cc -g -o $@ $^

%.o : %.c
	cc -g -c -o $@ $<

clean : 
	rm -vf bin/${BIN} ${OBJ}

run :
	bin/${BIN} /mnt/f/cli/cli/bin/Debug/cli.exe

