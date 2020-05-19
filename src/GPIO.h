#pragma once

#include <cstdint>
#include "noncopyable.h"

namespace gpio {

class OUT : noncopyable {
public:
    explicit OUT(uint8_t pin);

public:
    void set(uint8_t val);
    uint8_t value() const;

    void toggle();

private:
    uint8_t _pin;
    bool _val;
};

}
