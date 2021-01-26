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
    enum Type {
        INTEGER,
        LONG,
        FLOAT,
        DOUBLE,
        OBJECT,
        STRING,
    };

    Value(int v) { type = INTEGER; value.i = v; }
    Value(long v) { type = LONG; value.l = v; }
    Value(float v) { type = FLOAT; value.f = v; }
    Value(double v) { type = DOUBLE; value.d = v; }
    Value(Object* v = 0) { type = OBJECT; value.o = v; }
    Value(const char* v) { type = STRING;  value.s = ref(v); }

    Value(const Value& v) {
        type = v.type;
        value = v.value;

        if (type == OBJECT) {
            ref(value.o);
        }
        else if (type == STRING) {
            ref(value.s);
        }
    }

    Value& operator = (const Value& v) {
        type = v.type;
        value = v.value;

        if (type == OBJECT) {
            ref(value.o);
        }
        else if (type == STRING) {
            ref(value.s);
        }

        return *this;
    }

    ~Value() {
        Reset();
    }

    void Reset() {
        if (type == OBJECT) {
            unref(value.o);
        }
        else if (type == STRING) {
            unref_a(value.s);
        }

        type = OBJECT;
        value.o = 0;
    }

    int GetType() const {
        return type;
    }

    bool IsZero() const {
        if (type == INTEGER) return value.i == 0;
        if (type == LONG) return value.l == 0;
        if (type == FLOAT) return value.f == 0;
        if (type == DOUBLE) return value.d == 0;
        if (type == OBJECT) return value.o == 0;
        if (type == STRING) return value.s == 0;
        assert(false);
        return true;
    }

    long ToInterger() const {
        if (type == INTEGER) return value.i;
        if (type == LONG) return value.l;
        assert(false);
        return 0;
    }

    double ToNumber() const {
        if (type == INTEGER) return value.i;
        if (type == LONG) return value.l;
        if (type == FLOAT) return value.f;
        if (type == DOUBLE) return value.d;
        assert(false);
        return 0;
    }

    const char * ToString(char *c ) const {
        if (type == INTEGER) {
            snprintf(c, 256, "%d", value.i);
        }
        else if (type == LONG) {
            snprintf(c, 256, "%ld", value.l);
        }
        else if (type == FLOAT) {
            snprintf(c, 256, "%f", value.f);
        }
        else if (type == DOUBLE) {
            snprintf(c, 256, "%lf", value.d);
        }
        else if (type == OBJECT) {
            snprintf(c, 256, "%p", value.o);
        }
        else {
            snprintf(c, 256, "%s", value.s);
        }
        
        return c;
    }

    const char* ToStr() const {
        if (type == STRING) return value.s;
        assert(false);
        return 0;
    }

    static Value Nil;
};

const Value operator + (const Value& a, const Value& b);
const Value operator - (const Value& a, const Value& b);
const Value operator * (const Value& a, const Value& b);
const Value operator / (const Value& a, const Value& b);
const Value operator % (const Value& a, const Value& b);
const Value operator - (const Value& a);

const Value operator & (const Value& a, const Value& b);
const Value operator | (const Value& a, const Value& b);

