#pragma once

#include <assert.h>


int ref(long key);
int unref(long key);

template<class T>
T* ref(T* ptr) {
    if (ptr == 0) {
        return ptr;
    }

    ref((long)ptr);

    return ptr;
}

template<class T>
void unref_a(T* ptr) {
    if (ptr != 0 && unref((long)ptr) == 0) {
        delete[] ptr;
    }
}

template<class T>
void unref(T* ptr) {
    if (ptr != 0 && unref((long)ptr) == 0) {
        delete ptr;
    }
}