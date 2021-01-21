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
    std::vector<Instruction> instructions;

public:
    Method(int64_t key, int argCount) : Member(key) {
        this->argCount = argCount;
    }

    void AddInstruction(Code opcode, int64_t oprand);
    Instruction GetInstruction(int i) const;

    virtual int Begin(Process* process)  const;
};