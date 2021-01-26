#pragma once

#include <vector>

#include "value.h"

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