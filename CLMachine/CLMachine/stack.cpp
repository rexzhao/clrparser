#include "stack.h"

#include <iostream>

int Stack::GetTop() const {
    return (int)_values.size();
}

void Stack::SetTop(int n) {
    _values.resize(n);
}

const Value& Stack::operator[](int index) const {
    int top = GetTop();

    if (index >= 0 && index < (int)_values.size()) {
        return _values[index];
    }
    else if (index < 0 && index >= -top) {
        return _values[top + index];
    }
    return Value::Nil;
}

#define DEBUG_STACK(T)  // do {char c[256]; std::cout << "  " << #T << " " << value.ToString(c) << ", size " << _values.size() << std::endl; } while(0)

void Stack::Push(const Value& value) {
    DEBUG_STACK(push);

    _values.push_back(value);
}

const Value Stack::Pop() {
    assert(!_values.empty());

    Value value = _values.back();
    _values.pop_back();

    DEBUG_STACK(pop);

    return value;
}

#undef DEBUG_STACK

void Stack::Exchange(IStack& target, int n) {
    int top = GetTop();

    int start = top - n;
    if (start < 0) {
        start = 0;
    }

    for (int i = start; i < top; i++) {
        target.Push(_values[i]);
    }

    _values.resize(start);

    top = start;
}