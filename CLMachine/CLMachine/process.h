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

        IStack* stack;
        std::vector<Value> locals;
        int pc;
        int ret;

        Frame(const Method* method, IStack* stack) 
            : method(method), stack(stack), pc(0), ret(0), locals(4) {
        }

        ~Frame() {
            delete stack;
        }
    };

    std::list<Frame*> frames;


    Frame * cur;
    Stack stack;

    void StoreLocal(int pos, const Value& obj);
    void Return();

    void CallMethod(int64_t key);

public:
    Process(const Context* context)
        : context(context), cur(0) {
    }

    ~Process() {
        for (auto ite = frames.begin(); ite != frames.end(); ite++) {
            delete* ite;
        }
    }

    void PushFrame(const Method* method, IStack * stack) {
        if (cur != NULL) {
            frames.push_back(cur);
        }

        cur = new Frame(method, stack);
        cur->pc = 0;
    }

    const Context* GetContext() {
        return context;
    }

    IStack* GetBaseStack() {
        return &stack;
    }

    IStack* GetStack() {
        if (cur == NULL) {
            return &stack;
        }
        return cur->stack;
    }

    void Start(const IMethod* method);
    bool Step();
};
