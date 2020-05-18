#pragma once

#include <Arduino.h>
#include "noncopyable.h"

class Relay : noncopyable {
public:
    explicit Relay(uint8_t pin);

public:
    void on() const;
    void off() const;
    void set(bool state) const;

private:
    uint8_t _pin;
};
