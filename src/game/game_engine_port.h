#pragma once

#include <cstdint>
#include <cstddef>

namespace ge {

struct Canvas {
    virtual void onClear() = 0;

    virtual void onDraw() = 0;

    virtual void* native() { return nullptr; }

    virtual void drawBitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint16_t w, uint16_t h, uint16_t color) = 0;

    virtual size_t drawText(uint16_t x, uint16_t y, const char* fmt, ...) = 0;
};

unsigned long nowMs();

unsigned long nowUs();

void delayUs(unsigned int us);

}
