#include "stack.h"

#include <iostream>

int Stack::GetTop() const {
    return (int)_values.size() - base;
}

void Stack::SetTop(int n) {
    _values.resize(base + n);
}


Value* Stack::operator[](int index) {
    int top = GetTop();

    if (index >= 0 && index < top) {
        return _values.get(base + index);
    }
    else if (index < 0 && index >= -top) {
        return _values.get(base + top + index);
    }
    return NULL;
}

void Stack::Set(int index, Value * value) {
    int top = GetTop();

    Value* v = NULL;
    if (index >= 0 && index < top) {
        _values.set(base + index, value);
    }
    else if (index < 0 && index >= -top) {
        _values.set(base + top + index, value);
    }
    else {
        assert(v != NULL);
    }
}

#define DEBUG_STACK(T)  // do {char c[256]; std::cout << "  " << #T << " " << value.ToString(c) << ", size " << _values.size() << std::endl; } while(0)

void Stack::Push(Value * value) {
    DEBUG_STACK(push);

    _values.push(value);
}

Value * Stack::Pop() {
    assert(_values.size() > base);

    DEBUG_STACK(pop);

    return _values.pop();
}

#undef DEBUG_STACK

void Stack::Clear() {
    _values.resize(base);
}

void Stack::SetBase(int n) {
    base = n;
}