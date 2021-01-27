#pragma once

#include <vector>
#include <stdlib.h>

#include "value.h"

void init_stack(struct SimpleStack* stack, int size);
void grow_stack(struct   SimpleStack* stack, int size);

struct SimpleStack {
    Value* head;
    int size;

    Value* top;
    Value* base;

    inline void push(Value* value) {
        if (top == head + size) {
            grow_stack(this, 1);
        }
        *top = *value;
        top++;
    }

    inline Value* pop() {
        return --top;
    }

    inline Value* get(int index) {
        return base + index;
    }
};


