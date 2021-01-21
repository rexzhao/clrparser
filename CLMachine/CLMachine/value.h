#pragma once

#include <assert.h>
#include <stdio.h>

#include "memory.h"

class Object {

};

class Value {
    int type;

    union {
        int i;
        long l;
        double d;
        float f;
        Object* o;
        const char* s;
    } value;
public:
    Value(int v) { type = 0; value.i = v; }
    Value(long v) { type = 1; value.l = v; }
    Value(float v) { type = 2; value.f = v; }
    Value(double v) { type = 3; value.d = v; }
    Value(Object* v = 0) { type = 4; value.o = v; }
    Value(const char* v) { type = 5;  value.s = ref(v); }

    Value(const Value& v) {
        type = v.type;
        if (v.type == 4) {
            value.o = ref(v.value.o);
        }
        else if (v.type == 5) {
            value.s = ref(v.value.s);
        }
        else {
            value.l = v.value.l;
        }
    }

    Value& operator = (const Value& v) {
        type = v.type;
        if (v.type == 4) {
            value.o = ref(v.value.o);
        }
        else if (v.type == 5) {
            value.s = ref(v.value.s);
        }
        else {
            value.l = v.value.l;
        }
        return *this;
    }

    ~Value() {
        Reset();
    }

    void Reset() {
        if (type == 4) {
            unref(value.o);
        }
        else if (type == 5) {
            unref_a(value.s);
        }

        type = 4;
        value.o = 0;
    }

    int GetType() const {
        return type;
    }

    bool IsZero() const {
        if (type == 0) return value.i == 0;
        if (type == 1) return value.l == 0;
        if (type == 2) return value.f == 0;
        if (type == 3) return value.d == 0;
        if (type == 4) return value.s == 0;
        if (type == 5) return value.o == 0;
        assert(false);
        return true;
    }

    long ToInterger() const {
        if (type == 0) return value.i;
        if (type == 1) return value.l;
        assert(false);
        return 0;
    }

    double ToNumber() const {
        if (type == 0) return value.i;
        if (type == 1) return value.l;
        if (type == 2) return value.f;
        if (type == 3) return value.d;
        assert(false);
        return 0;
    }

    const char * ToString(char *c ) const {
        if (type == 0) {
            snprintf(c, 256, "%d", value.i);
        }
        else if (type == 1) {
            snprintf(c, 256, "%ld", value.l);
        }
        else if (type == 2) {
            snprintf(c, 256, "%f", value.f);
        }
        else if (type == 3) {
            snprintf(c, 256, "%lf", value.d);
        }
        else if (type == 4) {
            snprintf(c, 256, "%p", value.o);
        }
        else {
            snprintf(c, 256, "%s", value.s);
        }
        
        return c;
    }

    const char* ToStr() const {
        if (type == 5) return value.s;
        assert(false);
        return 0;
    }

    static Value Nil;
};

const Value operator + (const Value& a, const Value& b);
const Value operator - (const Value& a, const Value& b);
const Value operator * (const Value& a, const Value& b);
const Value operator / (const Value& a, const Value& b);