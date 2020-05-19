#pragma once

#include <Arduino.h>
#include "GPIO.h"

#define code

extern gpio::OUT IO_SCL;
extern gpio::OUT IO_SDA;
extern gpio::OUT IO_RST;

#define SCL IO_SCL
#define SDA IO_SDA
#define RST IO_RST

#define DelayMs delay
