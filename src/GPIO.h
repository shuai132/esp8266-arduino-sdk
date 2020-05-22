#pragma once

#include <cstdint>
#include "noncopyable.h"

namespace gpio {

class OUT : noncopyable {
public:
    explicit OUT(uint8_t pin, uint8_t val = LOW);

public:
    void set(uint8_t val);
    uint8_t value(bool read = true);

    void toggle();

    OUT& operator=(uint8_t val);

private:
    uint8_t _pin;
    uint8_t _val;
};

}
