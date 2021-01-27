#include "memory.h"

#include <map>

/*
static std::map<uint64_t, int> memorys;
int ref(uint64_t key) {
	return ++memorys[key];
}

int unref(uint64_t key) {
	int c = memorys[key];
	assert(c >= 1);

	if (c == 1) {
		memorys.erase(key);
		return 0;
	} 

	return --memorys[key];
}
*/

#define PTR_HEAD(ptr) (((uint32_t*)(ptr)) - 1)
#define PTR_REF(ptr) (*(((uint32_t*)(ptr)) - 1))

void* Alloc(void* ptr, size_t size, void* u) {
	if (ptr) {
		return realloc(ptr, size);
	} else {
		return malloc(size);
	}
/*
	size = size + sizeof(uint32_t);
	if (ptr == NULL) {
		uint32_t * mem = (uint32_t*)malloc(size);
		if (mem == NULL) return NULL;

		ptr = mem + 1;

		mem[0] = 0;

		return ptr;
	}

	uint32_t* mem = PTR_HEAD(ptr);
	uint32_t* new_ptr = (uint32_t*)realloc(mem, size);
	if (new_ptr == NULL) {
		free(mem);
		return NULL;
	}

	return new_ptr + 1;
*/
}


void Free(void* ptr) {
	free(ptr);

	// free(PTR_HEAD(ptr));
}

void* Ref(void* ptr) {
	// PTR_REF(ptr)++;
	return ptr;
}

int Unref(void* ptr) {
	return 1;
	// return --PTR_REF(ptr);
}
