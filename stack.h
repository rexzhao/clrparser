#ifndef _VM_STACK_H_
#define _VM_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif


struct Stack {
	int top;
	int size;
	void ** slots;
};

void Stack_Init(struct Stack * stack);

void Stack_Grow(struct Stack * stack);
void Stack_Push(struct Stack * stack, void * value);
void Stack_Pop(struct Stack * stack, int n);

void * Stack_Get(struct Stack * stack, int index);
void Stack_Move(struct Stack * from, struct Stack * to, int count);

void Stack_Release(struct Stack * stack);


#ifdef __cplusplus
}
#endif

#endif
