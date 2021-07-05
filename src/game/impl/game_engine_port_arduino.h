#include "game/game_engine_port.h"
#include <Arduino.h>
#include "OLED.h"

struct OLEDScreen : public ge::Canvas {
    OLEDScreen() {
        display.begin();
    }

    void onClear() override {
        display.clearDisplay();
    }

    void onDraw() override {
        display.display();
    }

    void* native() override {
        return &display;
    }

    void drawBitmap(uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t w, uint16_t h, uint16_t color) override {
        display.drawBitmap(x, y, bitmap, w, h, color);
    }

    size_t drawText(uint16_t x, uint16_t y, const char* format, ...) override {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(x, y);

        va_list arg;
        va_start(arg, format);
        char temp[64];
        char* buffer = temp;
        size_t len = vsnprintf(temp, sizeof(temp), format, arg);
        va_end(arg);
        if (len > sizeof(temp) - 1) {
            buffer = new (std::nothrow) char[len + 1];
            if (!buffer) {
                return 0;
            }
            va_start(arg, format);
            vsnprintf(buffer, len + 1, format, arg);
            va_end(arg);
        }
        len = display.write((const uint8_t*) buffer, len);
        if (buffer != temp) {
            delete[] buffer;
        }
        return len;
    }
};
