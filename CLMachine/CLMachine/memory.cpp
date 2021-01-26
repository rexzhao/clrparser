#include "memory.h"

#include <map>

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