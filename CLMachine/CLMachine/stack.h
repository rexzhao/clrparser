#pragma once

#include <vector>

#include "value.h"

class IStack {
public:
    virtual ~IStack() {}

    virtual int GetTop() const = 0;
    virtual void SetTop(int n) = 0;

    virtual const Value& operator[](int index) const = 0;

    const Value& Get(int index) {
        return this->operator[](index);
    }

    virtual void Push(const Value& value) = 0;
    virtual const Value Pop() = 0;
    virtual void Exchange(IStack& target, int n) = 0;    
};

class Stack : public IStack {
    std::vector<Value> _values;
public:
    virtual int GetTop() const;
    virtual void SetTop(int n);

    virtual const Value& operator[](int index) const;

    virtual void Push(const Value& value);
    virtual const Value Pop();
    virtual void Exchange(IStack& target, int n);
};



class WarpStack : public IStack {
    int base;
    IStack* stack;

public:
    WarpStack(IStack* stack, int n = 0) {
        this->stack = stack;

        base = this->stack->GetTop() - n;

        assert(base >= 0);
    }

    ~WarpStack() {
        stack->SetTop(base);
    }

    virtual int GetTop() const {
        return stack->GetTop() - base;
    }

    virtual void SetTop(int n) {
        stack->SetTop(base + n);
    }

    virtual void Push(const Value& value) {
        stack->Push(value);
    }

    virtual const Value Pop() {
        if (stack->GetTop() > base) {
            return stack->Pop();
        }
        return Value::Nil;
    }

    virtual void Exchange(IStack& target, int n) {
        int top = GetTop();
        if (n > top) {
            n = top;
        }
        stack->Exchange(target, n);
    }

    virtual const Value& operator[](int index) const {
        if (index >= 0) {
            return stack->operator[](base + index);
        }
        else if (index >= base - stack->GetTop()) {
            return stack->operator[](index);
        }
        return Value::Nil;
    }
};
