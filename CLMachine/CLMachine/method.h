#pragma once

#include <vector>

#include "member.h"
#include "code.h"
#include "process.h"

class context;

struct Instruction {
    Code opcode;
    int64_t oprand;

    Instruction(Code opcode, int64_t oprand) {
        this->opcode = opcode;
        this->oprand = oprand;
    }

    static Instruction ret;
};

class IMethod {
public:
    virtual int Begin(Process * process) const = 0;
    virtual ~IMethod() {}

    virtual Instruction* GetInstruction(int i) const = 0;

    virtual void Dump(Process* process, int pc) const {}
};

class NativeMethod : public IMethod {
    int (*func)(Process * p);

public:
    NativeMethod(int (*func)(Process* p)) {
        this->func = func;
    }

    virtual int Begin(Process* p)  const {
        return func(p);
    }

    virtual Instruction* GetInstruction(int i) const {
        return &Instruction::ret;
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

    virtual Instruction * GetInstruction(int i) const;
    virtual int Begin(Process* p)  const;

    void Dump(Process* process, int pc) const;
    void DumpInstruction(Process* process, const Instruction& instruction) const;
};