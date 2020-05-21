#include <Arduino.h>
#include "GPIO.h"

namespace gpio {

OUT::OUT(uint8_t pin)
    : _pin(pin) {
    pinMode(_pin, OUTPUT);
}

void OUT::set(uint8_t val) {
    _val = val;
    digitalWrite(_pin, _val);
}

uint8_t OUT::value() const {
    return _val;
}

void OUT::toggle() {
    set(!_val);
}

OUT& OUT::operator=(uint8_t val) {
    set(val);
    return *this;
}

}
