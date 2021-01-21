#ifndef _VM_INSTRUCTION_H_
#define _VM_INSTRUCTION_H_

#ifdef __cplusplus
extern "C" {
#endif

struct MethodBody;
struct Instruction;

typedef void (*Instruction_DO)(struct Instruction * instruction, int pc, struct MethodBody * body);

struct Instruction {
	Instruction_DO DO;
};

#ifdef __cplusplus
}
#endif

#endif
