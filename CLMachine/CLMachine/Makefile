OBJ=$(patsubst %.cpp, %.o, $(wildcard *.cpp))

BIN=clm

all : ${BIN}

${BIN} : ${OBJ}
	g++ -g -o $@ $^

%.o : %.c
	g++ -g -c -o $@ $<

clean : 
	rm -vf ${BIN} ${OBJ}

run :
	${BIN} a.dll
