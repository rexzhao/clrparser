#include "memory.h"

#include <map>

static std::map<long, int> memorys;


int ref(long key) {
	return ++memorys[key];
}

int unref(long key) {
	int c = memorys[key];
	assert(c >= 1);

	if (c == 1) {
		memorys.erase(key);
		return 0;
	} 

	return --memorys[key];
}