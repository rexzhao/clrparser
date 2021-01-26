#pragma once

#include <stdint.h>
#include <list>

#include "value.h"
#include "stack.h"

class Context;
class IMethod;
class Method;

class Process {
    const Context* context;

    struct Frame {
        const Method* method;

        int local;
        int stack;

        int pc;
        int ret;

        Frame() : method(0), local(0), stack(0), pc(0), ret(0) {};

        void Reset() {
            method = 0;
            local = stack = pc = ret = 0;
        }
    };

    std::list<Frame> frames;


    Frame cur;
    Stack stack;
    Stack locals;

    void StoreLocal(int pos, const Value& obj);
    void Return();

    void CallMethod(int64_t key);

public:
    Process(const Context* context)
        : context(context) {
    }

    ~Process() {
    }

    void PushFrame(const Method* method, int argCount);

    const Context* GetContext() {
        return context;
    }

    /*
    IStack* GetBaseStack() {
        return &stack;
    }
    */
    IStack* GetStack() {
        return &stack;
    }

    void Start(const IMethod* method);
    bool Step();
};
