#include "game_engine_port_arduino.h"
#include <Arduino.h>

namespace ge {

unsigned long nowMs() {
    return micros();
}
unsigned long nowUs() {
    return millis();
}
void delayUs(unsigned int us) {
    delay(us);
}

}
