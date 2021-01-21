#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "value.h"
#include "instruction.h"
#include "method_body.h"

struct I_Call {
	Instruction_DO DO;

	int paramCount;
	int retCount;

	struct Instruction * instructions;
};

static void I_Call_DO(struct Instruction * _instruction, int pc, struct MethodBody * body) { 
	struct I_Call * instruction = (struct I_Call*)_instruction;

	struct MethodBody newBody;

	MethodBody_Init(&newBody, instruction->instructions);

	int paramCount = instruction->paramCount;
	int retCount = instruction->retCount;

	Stack_Move(&body->stack, &newBody.stack, paramCount);

	while(!MethodBody_IsFinished(&newBody)) {
		MethodBody_Invoke(&newBody);
	}

	Stack_Move(&newBody.stack, &body->stack, retCount);

	MethodBody_Release(&newBody);
}

struct I_LoadField {
	Instruction_DO DO;

	int field;
};

static void I_LoadField_DO(struct Instruction * _instruction, int pc, struct MethodBody * body) { 
	struct I_LoadField * instruction = (struct I_LoadField*)_instruction;

	int field = instruction->field;
	struct Stack * stack = &body->stack;

	struct Value * target = (struct Value*)Stack_Get(stack, -1);
	Stack_Pop(stack, 1);

	Stack_Push(stack, target->v.fields[field].value);
}

struct I_SaveField {
	Instruction_DO DO;

	int field;
};

static void I_SaveField_DO(struct Instruction * _instruction, int pc, struct MethodBody * body) { 
	struct I_SaveField * instruction = (struct I_SaveField*)_instruction;

	int field = instruction->field;
	struct Stack * stack = &body->stack;

	struct Value * value  = (struct Value*)Stack_Get(stack, -1);
	struct Value * target = (struct Value*)Stack_Get(stack, -2);

	Stack_Pop(stack, 2);

	target->v.fields[field].value = value;
}
