
struct OpCode {
	const char * name;
	unsigned char code[2];
	int oprand;
};

extern struct OpCode opCodes [];
 

const char * dump_opcode(const char * ptr);
