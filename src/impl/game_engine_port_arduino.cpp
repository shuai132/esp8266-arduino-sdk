#include "game_engine_port_arduino.h"
#include <Arduino.h>

namespace ge {

unsigned long nowMs() {
    return millis();
}
unsigned long nowUs() {
    return micros();
}
void delayUs(unsigned int us) {
    delayMicroseconds(us);
}

bool checkButton() {
    return !digitalRead(0);
}

}
