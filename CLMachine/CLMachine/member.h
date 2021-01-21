#pragma once

#include <stdint.h>

class Member {
protected:
    int64_t key;

public:
    Member(int64_t key) {
        this->key = key;
    }

    int64_t GetKey() {
        return key;
    }
};
