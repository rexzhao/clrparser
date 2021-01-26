#pragma once

#include <vector>

#include "value.h"

template<class T>
class Array {
    T* ptr;
    int size;
    int cap;

    void grow() {
        cap *= 2;
        if (ptr == 0) {
            ptr = (T*)malloc(sizeof(T) * cap);
        }
        else {
            ptr = (T*)realloc(ptr, sizeof(T) * cap);
        }
    }

public:
    Array() {
        size = 0;
        ptr = 0;
        cap = 2;
        grow();
    }

    int Size() {
        return size;
    }

    void Set(int index, T* v) {
        assert(index <= size);
        ptr[index] = *v;
    }

    T * Push(T v) {
        if (size == cap) {
            grow();
        }

        ptr[size++] = v;

        return ptr + (size - 1);
    }

    T* Pop() {
        return ptr + (--size);
    }

    T* Last() {
        if (size <= 0) return NULL;

        return ptr + (size - 1);
    }

    T* Get(int index) {
        return ptr + index;
    }
};

class IStack {
public:
    virtual ~IStack() {}

    virtual int GetTop() const = 0;
    virtual void SetTop(int n) = 0;

    virtual Value& operator[](int index) = 0;

    const Value& Get(int index) {
        return this->operator[](index);
    }

    virtual void Push(const Value& value) = 0;
    virtual const Value Pop() = 0;
};

class Stack : public IStack {
    std::vector<Value> _values;

    int base;

public:
    Stack() : base(0) {};

    virtual int GetTop() const;
    virtual void SetTop(int n);

    virtual Value& operator[](int index);

    virtual void Push(const Value& value);
    virtual const Value Pop();
    // virtual void Exchange(IStack& target, int n);

    void SetBase(int n);
    void Clear();
};