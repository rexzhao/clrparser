#pragma once

#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "memory.h"

class Object {

};

struct Value {
    int type;

    union {
        long i;
        double d;
        void* p;
    } value;

    enum Type {
        INTEGER,
        NUMBER,
        POINTER,
    };

    Value(int v) { type = INTEGER; value.i = v; }
    Value(long v) { type = INTEGER; value.i = v; }

    Value(float v) { type = NUMBER; value.d = v; }
    Value(double v) { type = NUMBER; value.d = v; }
    Value(void * v = 0) { type = POINTER; value.p = v; }
    Value(const char * v = 0) { type = POINTER; value.p = (void*)v; }
    Value(char* v = 0) { type = POINTER; value.p = (void*)v; }

    /*
    Value(const Value& v) {
        Release();

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
        Release();

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
        Release();
    }
    */
    void Release() {
        type = POINTER;
        value.p = 0;
    }


    inline int GetType() const {
        return type;
    }

    inline bool IsZero() const {
        if (type == INTEGER) return value.i == 0;
        if (type == NUMBER) return value.d == 0;
        assert(false);
        return true;
    }

    inline long ToInterger() const {
        if (type == INTEGER) return value.i;
        assert(false);
        return 0;
    }

    inline double ToNumber() const {
        if (type == INTEGER) return value.i;
        if (type == NUMBER) return value.d;
        assert(false);
        return 0;
    }

    const char * ToString(char *c ) const {
        if (type == INTEGER) {
            snprintf(c, 256, "%ld", value.i);
        }
        else if (type == NUMBER) {
            snprintf(c, 256, "%lf", value.d);
        }
        else if (type == POINTER) {
            snprintf(c, 256, "%s", (const char*)value.p);
        }

        return c;
    }

    const char* ToStr() const {
        if (type == POINTER) return (const char*)value.p;
        assert(false);
        return 0;
    }

    static Value Nil;

#define VALUE_MATH(a, o, b) \
do { \
    int t = (a->type < b->type) ? b->type : a->type; \
    if (t == Value::INTEGER) { \
        a->type = Value::INTEGER; \
        a->value.i = a->value.i o b->value.i; \
    } \
    else if (t == Value::NUMBER) { \
        a->type = Value::NUMBER; \
        a->value.d = a->ToNumber() o b->ToNumber(); \
    } \
    else { \
        assert(false); \
    } \
} while(0)


#define VALUE_BIT(a, o, b) \
do { \
    assert(a->GetType() == b->GetType()); \
    int t = a->GetType(); \
    if (t == Value::INTEGER) { \
        a->value.i = a->value.i o b->value.i; \
    } else { \
        assert(false); \
    } \
} while(0)


    inline Value* Add(Value* v) {
        VALUE_MATH(this, +, v);
        return this;
    }

    inline Value* Sub(Value* v) {
        VALUE_MATH(this, -, v);
        return this;
    }
    inline Value* Mul(Value* v) {
        VALUE_MATH(this, *, v);
        return this;
    }
    inline Value* Div(Value* v) {
        VALUE_MATH(this, / , v);
        return this;
    }
    inline Value* Rem(Value* v) {
        Value* a = this;
        Value* b = v;
        int t = (a->type < b->type) ? b->type : a->type;
        if (t == Value::INTEGER) {
            a->type = INTEGER;
            a->value.i = a->value.i % b->value.i;
        }
        else if (t == Value::NUMBER) {
            a->type = Value::NUMBER;
            a->value.d = fmod(a->ToNumber(), b->ToNumber());
        }
        else {
            assert(false);
        }

        return this;
    }

    inline Value* And(Value* v) {
        VALUE_BIT(this, &, v);
        return this;
    }

    inline Value* Or(Value* v) {
        VALUE_BIT(this, | , v);
        return this;
    }

    inline Value* Neg() {
        if (this->type == Value::INTEGER) {
            this->value.i = -this->value.i;
        }
        else if (this->type == Value::NUMBER) {
            this->value.d = -this->value.d;
        }
        else {
            assert(false);
        }


        return this;
    }
};

