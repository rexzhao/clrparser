#include "value.h"


#include <limits>
#include <math.h>


Value Value::Nil;


#define VALUE_MATH(a, o, b) \
do { \
    int t = (a->GetType() < b->GetType()) ? b->GetType() : a->GetType(); \
    if (t == Value::INTEGER) { \
        long sum = a->ToInterger() o b->ToInterger(); \
        if (sum <= std::numeric_limits<int>::max() && sum >= std::numeric_limits<int>::min()) { \
            this->type = INTEGER; \
            this->value.i = (int)sum; \
        } \
        else { \
            this->type = LONG; \
            this->value.l = sum; \
        } \
    } \
    else if (t == Value::LONG) { \
        this->value.l = a->ToInterger() o b->ToInterger(); \
    } \
    else if (t == Value::FLOAT) { \
        double sum = a->ToNumber() o b->ToNumber(); \
        if (sum <= std::numeric_limits<float>::max() && sum >= std::numeric_limits<float>::min()) { \
            this->type = FLOAT; \
            this->value.f = (float)sum; \
        } \
        else { \
            this->type = DOUBLE; \
            this->value.d = (float)sum; \
        } \
    } \
    else if (t == Value::DOUBLE) { \
        this->type = DOUBLE; \
        this->value.d = a->ToNumber() o b->ToNumber(); \
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
    } \
    else if (t == Value::LONG) { \
        a->value.l = a->value.l o b->value.l; \
    } else { \
        assert(false); \
    } \
} while(0)


Value* Value::Add(Value* v) {
    VALUE_MATH(this, +, v);
    return this;
}

Value* Value::Sub(Value* v) {
    VALUE_MATH(this, -, v);
    return this;
}
Value* Value::Mul(Value* v) {
    VALUE_MATH(this, *, v);
    return this;
}
Value* Value::Div(Value* v) {
    VALUE_MATH(this, /, v);
    return this;
}
Value* Value::Rem(Value* v) {
    Value* a = this;
    Value* b = v;
    int t = (a->GetType() < b->GetType()) ? b->GetType() : a->GetType();
    if (t == Value::INTEGER) {
        long sum = a->ToInterger() % b->ToInterger();
        if (sum <= std::numeric_limits<int>::max() && sum >= std::numeric_limits<int>::min()) {
            this->type = INTEGER;
            this->value.i = (int)sum;
        }
        else {
            this->type = LONG;
            this->value.l = sum;
        }
    }
    else if (t == Value::LONG) {
        this->value.l = a->ToInterger() % b->ToInterger();
    }
    else if (t == Value::FLOAT) {
        double sum = fmod(a->ToNumber(), b->ToNumber());
        if (sum <= std::numeric_limits<float>::max() && sum >= std::numeric_limits<float>::min()) {
            this->type = FLOAT;
            this->value.f = (float)sum;
        }
        else {
            this->type = DOUBLE;
            this->value.d = (float)sum;
        }
    }
    else if (t == Value::DOUBLE) {
        this->type = DOUBLE;
        this->value.d = fmod(a->ToNumber(), b->ToNumber());
    }
    else {
        assert(false);
    }

    return this;
}

Value* Value::And(Value* v) {
    VALUE_BIT(this, &, v);
    return this;
}
Value* Value::Or(Value* v) {
    VALUE_BIT(this, |, v);
    return this;
}
Value* Value::Neg() {
    return this;
}