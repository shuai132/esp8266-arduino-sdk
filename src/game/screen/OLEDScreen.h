#pragma once

#include "../game_engine.hpp"
#include "OLED.h"

struct OLEDScreen : public Screen {
    OLEDScreen() {
        display.begin();
        // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
        if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
            Serial.println(F("SSD1306 allocation failed"));
            for(;;); // Don't proceed, loop forever
        }
    }
    void onClear() override {
        display.clearDisplay();
    }
    void onDraw() override {
        display.display();
    }
    Canvas * getCanvas() override {
        return &display;
    }
};
