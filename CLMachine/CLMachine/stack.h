#pragma once

#include <vector>

#include "value.h"

template<class T>
class Array {
    T* ptr;
    int _size;
    int _cap;

    void grow() {
        _cap *= 2;
        if (ptr == 0) {
            ptr = (T*)malloc(sizeof(T) * _cap);
        }
        else {
            ptr = (T*)realloc(ptr, sizeof(T) * _cap);
        }
    }

public:
    Array() {
        _size = 0;
        ptr = 0;
        _cap = 2;
        grow();
    }

    int size() const {
        return _size;
    }

    void resize(int n) {
        if (n <= _size) {
            _size = n;
        }
        else {
            while (_cap < n) {
                grow();
            }
            _size = n;
        }
    }

    void set(int index, T * v) {
        assert(index <= _size);
        ptr[index] = *v;
    }

    T * push(T * v) {
        if (_size == _cap) {
            grow();
        }

        ptr[_size++] = *v;

        return ptr + (_size - 1);
    }

    T* pop() {
        return ptr + (--_size);
    }

    T* last() {
        if (_size <= 0) return NULL;

        return ptr + (_size - 1);
    }

    T* get(int index) {
        return ptr + index;
    }
};

class IStack {
public:
    virtual ~IStack() {}

    virtual int GetTop() const = 0;
    virtual void SetTop(int n) = 0;

    virtual Value* operator[](int index) = 0;

    Value * Get(int index) {
        return (*this)[index];
    }

    virtual void Set(int index, Value * value) = 0;

    virtual void Push(Value * value) = 0;
    virtual Value * Pop() = 0;


    template<class T>
    void Push(T ptr) {
        Value v(ptr);
        Push(&v);
    }
};

class Stack : public IStack {
    Array<Value> _values;
    // std::vector<Value> _values;

    int base;

public:
    Stack() : base(0) {};

    virtual int GetTop() const;
    virtual void SetTop(int n);

    virtual Value * operator[](int index);
    virtual void Set(int index, Value * value);

    virtual void Push(Value* value);
    virtual Value * Pop();
    // virtual void Exchange(IStack& target, int n);

    void SetBase(int n);
    void Clear();
};