#pragma once

#include <assert.h>
#include <stdint.h>


int ref(uint64_t key);
int unref(uint64_t key);

template<class T>
T* ref(T* ptr) {
    if (ptr == 0) {
        return ptr;
    }

    ref((uint64_t)ptr);

    return ptr;
}

template<class T>
void unref_a(T* ptr) {
    if (ptr != 0 && unref((uint64_t)ptr) == 0) {
        delete[] ptr;
    }
}

template<class T>
void unref(T* ptr) {
    if (ptr != 0 && unref((uint64_t)ptr) == 0) {
        delete ptr;
    }
}