#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>


void* Alloc(void* ptr, size_t size, void * u);
void  Free(void* ptr);
void* Ref(void* ptr);
int   Unref(void* ptr);


template<class T>
T* ref(T* ptr) {
    if (ptr != 0) {
        Ref((void*)ptr);
    }
    return ptr;
}

template<class T>
void unref(T* ptr) {
    if (ptr != 0 && Unref((void*)ptr) == 0) {
        Free((void*)ptr);
    }
}
