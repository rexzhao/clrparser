#pragma once

#include <stdint.h>
#include <list>

#include "value.h"
#include "stack.h"

class Context;
class IMethod;
class Method;
struct Instruction;


struct CallInfo {
    const IMethod* method;

    int local;
    int localCount;

    int stack;
    int stackCount;
    
    int pc;
    int ret;
};


struct Process {
    const Context* context;

    CallInfo* base_ci;
    CallInfo* ci;
    int size_ci;

    SimpleStack local;
    SimpleStack stack;

    const IMethod* method;
    Instruction * pc;
    int ret;
};

void prepare_call(Process* p, const IMethod* method, int arg);
void run(const Context* context, int64_t key);