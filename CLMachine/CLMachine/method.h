#pragma once

#include <vector>

#include "member.h"
#include "code.h"
#include "process.h"

class context;

class IMethod {
public:
    virtual int Begin(Process * process) const = 0;
    virtual ~IMethod() {}
};

class NativeMethod : public IMethod {
    int (*func)(const Context* context, IStack* stack);
public:
    NativeMethod(int (*func)(const Context* context, IStack* stack)) {
        this->func = func;
    }

    virtual int Begin(Process* process)  const {
        return func(process->GetContext(), process->GetStack());
    }
};


struct Instruction {
    Code opcode;
    int64_t oprand;

    Instruction(Code opcode, int64_t oprand) {
        this->opcode = opcode;
        this->oprand = oprand;
    }
};

class Method : public Member, public IMethod {
    int argCount;
    Instruction * instructions;
    int instructinsCount;

public:
    Method(int64_t key, int argCount) : Member(key), instructions(0), instructinsCount(0){
        this->argCount = argCount;
    }

    void SetInstruction(Instruction* instructions, int count);
    Instruction * GetInstruction(int i) const;

    virtual int Begin(Process* process)  const;

    void Dump(Process* process, int pc) const;
    void DumpInstruction(Process* process, const Instruction& instruction) const;
};