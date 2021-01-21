#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "method_body.h"
#include "instruction.h"

void MethodBody_Init(struct MethodBody * body, struct Instruction * instructions)
{
	body->pc = 0;
	body->instructions = instructions;
	Stack_Init(&body->stack);
}

void MethodBody_Jump(struct MethodBody * body, int pc)
{
	body->pc = pc;
}

int MethodBody_IsFinished(struct MethodBody * body)
{
	return body->instructions == 0 || body->instructions[body->pc].DO == 0;
}

void MethodBody_Invoke(struct MethodBody * body)
{
	int pc = body->pc ++;
	struct Instruction * instruction = body->instructions + pc;
	instruction->DO(instruction, pc, body);
}

void MethodBody_Release(struct MethodBody * body)
{
	body->pc = 0;
	body->instructions = 0;
	Stack_Release(&body->stack);
}
