#include "Relay.h"

Relay::Relay(uint8_t pin)
    : _pin(pin) {
    pinMode(_pin, OUTPUT);
}

void Relay::on() const {
    digitalWrite(_pin, LOW);
}

void Relay::off() const {
    digitalWrite(_pin, HIGH);
}

void Relay::set(bool state) const {
    state ? on() : off();
}
