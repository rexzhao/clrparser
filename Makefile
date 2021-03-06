OBJ=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

BIN=cclr

all : bin/${BIN}

bin/${BIN} : ${OBJ}
	g++ -g -o $@ $^

%.o : %.c
	g++ -g -c -o $@ $<

clean : 
	rm -vf bin/${BIN} ${OBJ}

run :
	bin/${BIN} a.dll
