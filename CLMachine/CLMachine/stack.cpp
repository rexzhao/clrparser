#include "stack.h"

#include <iostream>

void init_stack(struct SimpleStack* stack, int size) {
    stack->head = (Value*)malloc(sizeof(Value) * size);
    stack->size = size;
    stack->base = stack->head;
    stack->top = stack->head;
}

void grow_stack(struct   SimpleStack* stack, int size) {
    int newSize = (size <= stack->size) ? (stack->size * 2) : (stack->size + size);
    Value* old = stack->head;
    stack->head = (Value*)realloc(stack->head, sizeof(Value) * newSize);
    stack->size = newSize;

    stack->base = stack->head + (stack->base - old);
    stack->top = stack->head + (stack->top - old);
}