#include "method.h"

#include "context.h"
#include "stack.h"
#include "process.h"

int Method::Begin(Process* p)  const
{
    p->PushFrame(this, new WarpStack(p->GetBaseStack(), argCount >> 1));
    return argCount & 1;
}


void Method::AddInstruction(Code opcode, int64_t oprand) {
    instructions.push_back(Instruction(opcode, oprand));
}

Instruction Method::GetInstruction(int i) const
{
    if (i < (int)instructions.size()) {
        return instructions[i];
    }
    return Instruction(Code::Nop, 0);
}