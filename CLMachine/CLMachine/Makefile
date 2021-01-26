OBJ=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

BIN=clm

CFLAG=-g
CFLAG=-O3

all : ${BIN}

${BIN} : ${OBJ}
	g++ ${CFLAG} -o $@ $^

%.o : %.cpp
	g++ ${CFLAG} -c -o $@ $<

clean : 
	rm -vf ${BIN} ${OBJ}

check :
	valgrind  --show-reachable=yes  --leak-check=full ./${BIN}

run :
	${BIN} a.dll
