#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"

void Stack_Init(struct Stack * stack)
{
	memset(stack, 0, sizeof(struct Stack));
}

void Stack_Grow(struct Stack * stack) 
{
	int size = 4;
	while(size <= stack->size) size *= 2;
	
	void ** slots =  (void**)realloc(stack->slots, sizeof(void **) * size);
	assert(slots);

	stack->slots = slots;
	stack->size  = size;
}

void Stack_Push(struct Stack * stack, void * value)
{
	if (stack->top >= stack->size) {
		Stack_Grow(stack);
	}

	stack->slots[stack->top] = value;
	stack->top ++;
}

void Stack_Pop(struct Stack * stack, int n)
{
	assert(stack->top >= n);
	stack->top -= n;
}

void * Stack_Get(struct Stack * stack, int index)
{
	if (index < 0) {
		index += stack->top;
	}

	if (index >= 0 && index < stack->top) {
		return stack->slots[index];
	}

	return 0;
}

void Stack_Release(struct Stack * stack)
{
	stack->top = stack->size = 0;
	if (stack->slots) {
		free(stack->slots);
	}
}

void Stack_Move(struct Stack * from, struct Stack * to, int count)
{
	int start = from->top - count;
	if (start < 0) {
		start = 0;
	}

	for (int i = 0; i < count; i++) {
		Stack_Push(to, Stack_Get(from, start + i));
	}

	Stack_Pop(from, count);
}
