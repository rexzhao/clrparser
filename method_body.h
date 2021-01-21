#ifndef _VM_METHOD_BODY_H_
#define _VM_METHOD_BODY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stack.h"

struct Instruction;

struct MethodBody {
    int pc;
    struct Stack stack;

    struct Instruction * instructions;
};

void MethodBody_Init(struct MethodBody * body, struct Instruction * instructions);
void MethodBody_Invoke(struct MethodBody * body);
void MethodBody_Jump(struct MethodBody * body, int pc);
int MethodBody_IsFinished(struct MethodBody * body);
void MethodBody_Release(struct MethodBody * body);

#ifdef __cplusplus
}
#endif

#endif
