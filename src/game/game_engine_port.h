#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <string.h>

namespace ge {

struct Canvas {
    virtual void onClear() = 0;

    virtual void onDraw() = 0;

    virtual void* native() { return nullptr; }

    virtual void drawBitmap(uint16_t x, uint16_t y, const uint8_t bitmap[], uint16_t w, uint16_t h, uint16_t color) = 0;

    virtual size_t drawBuffer(uint16_t x, uint16_t y, const char* buffer, size_t len) = 0;

    size_t drawString(uint16_t x, uint16_t y, const char* str) {
        return drawBuffer(x, y, str, strlen(str));
    }

    size_t drawText(uint16_t x, uint16_t y, const char* format, ...) {
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
        len = drawBuffer(x, y, buffer, len);
        if (buffer != temp) {
            delete[] buffer;
        }
        return len;
    }

};

unsigned long nowMs();

unsigned long nowUs();

void delayUs(unsigned int us);

bool checkButton();

}
