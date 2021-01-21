#include "value.h"


#include <limits>




class MathOperatorAdd {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 + t2;
    }
};

class MathOperatorSub {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 - t2;
    }
};

class MathOperatorMul {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 * t2;
    }
};

class MathOperatorDiv {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 / t2;
    }
};

class MathOperatorRem {
public:
    template<class T> T DO(T t1, T t2) const {
        return (T)fmod(t1, t2);
    }
};


template<typename MathOperator>
static const Value math_operator(const Value& a, const Value& b, const MathOperator& opt) {
    int t = (a.GetType() < b.GetType()) ? b.GetType() : a.GetType();
    if (t == 0) {
        long sum = opt.DO(a.ToInterger(), b.ToInterger());
        if (sum <= std::numeric_limits<int>::max() && sum >= std::numeric_limits<int>::min()) {
            return Value((int)sum);
        }
        else {
            return Value(sum);
        }
    }

    if (t == 1) {
        return Value(opt.DO(a.ToInterger(), b.ToInterger()));
    }

    if (t == 2) {
        double sum = opt.DO(a.ToNumber(), b.ToNumber());
        if (sum <= std::numeric_limits<float>::max() && sum >= std::numeric_limits<float>::min()) {
            return Value((float)sum);
        }
        else {
            return Value(sum);
        }
    }

    if (t == 3) {
        return Value(opt.DO(a.ToNumber(), b.ToNumber()));
    }
    assert(false);

    return Value::Nil;
}

class MathOperatorAnd {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 & t2;
    }
};


class MathOperatorOr {
public:
    template<class T> T DO(T t1, T t2) const {
        return t1 | t2;
    }
};


template<typename BitOperator>
static const Value bit_operator(const Value& a, const Value& b, const BitOperator& opt) {
    assert(a.GetType() == b.GetType());
    if (a.GetType() == 0) {
        return Value(opt.DO((int)a.ToInterger(), (int)b.ToInterger()));
    }
    else if (a.GetType() == 1) {
        return Value(opt.DO(a.ToInterger(), b.ToInterger()));
    }

    assert(false);

    return Value::Nil;
}


const Value operator + (const Value& a, const Value& b) {
    return math_operator(a, b, MathOperatorAdd());
}

const Value operator - (const Value& a, const Value& b) {
    return math_operator(a, b, MathOperatorSub());
}

const Value operator * (const Value& a, const Value& b) {
    return math_operator(a, b, MathOperatorMul());
}

const Value operator / (const Value& a, const Value& b) {
    return math_operator(a, b, MathOperatorDiv());
}

const Value operator % (const Value& a, const Value& b) {
    return math_operator(a, b, MathOperatorRem());
}

const Value operator & (const Value& a, const Value& b) {
    return bit_operator(a, b, MathOperatorRem());
}

const Value operator | (const Value& a, const Value& b) {
    return bit_operator(a, b, MathOperatorRem());
}

const Value operator - (const Value& a) {
    int t = a.GetType();
    if (t == 0) {
        return Value((int)-a.ToInterger());
    }
    else if (t == 1) {
        return Value(-a.ToInterger());
}
    else if (t == 2) {
        return Value((float)-a.ToNumber());
    }
    else if (t == 3) {
        return Value(-a.ToNumber());
    }
        
    assert(false);

    return Value::Nil;
}

Value Value::Nil;