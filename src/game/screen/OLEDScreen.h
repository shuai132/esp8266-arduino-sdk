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
    void onDraw(FrameBuffer* frameBuffer) override {
        display.clearDisplay();
        auto& fb = *frameBuffer;
        auto& height = fb.height;
        auto& width = fb.width;
        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                auto pixel = fb.buffer[h*width + w];
                display.drawPixel(w, h, pixel);
            }
        }
        display.display();
    }
};
