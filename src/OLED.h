#pragma once

#include <Adafruit_SSD1306.h>

#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
extern Adafruit_SSD1306 display;
