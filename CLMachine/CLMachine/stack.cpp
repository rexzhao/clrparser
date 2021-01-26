#include "stack.h"

#include <iostream>

int Stack::GetTop() const {
    return (int)_values.size() - base;
}

void Stack::SetTop(int n) {
    _values.resize(base + n);
}

Value& Stack::operator[](int index) {
    int top = GetTop();

    if (index >= 0 && index < top) {
        return _values[base + index];
    }
    else if (index < 0 && index >= -top) {
        return _values[base + top + index];
    }
    return Value::Nil;
}

#define DEBUG_STACK(T)  // do {char c[256]; std::cout << "  " << #T << " " << value.ToString(c) << ", size " << _values.size() << std::endl; } while(0)

void Stack::Push(const Value& value) {
    DEBUG_STACK(push);

    _values.push_back(value);
}

const Value Stack::Pop() {
    assert(_values.size() > base);

    Value value = _values.back();
    _values.pop_back();

    DEBUG_STACK(pop);

    return value;
}

#undef DEBUG_STACK

void Stack::Clear() {
    _values.resize(base);
}

void Stack::SetBase(int n) {
    base = n;
}