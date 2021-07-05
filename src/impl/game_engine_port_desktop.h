#include "game/game_engine_port.h"

#include <memory>
#include <curses.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define PIX_PER_CHAR 1

struct Screen : public ge::Canvas {
    WINDOW* window;

    Screen() {
        initscr();
        raw();
        noecho();
        curs_set(0);
    }

    void onClear() override {
        clear();
    }

    void onDraw() override {
        refresh();
    }

    void drawBitmap(uint16_t x, uint16_t y, const uint8_t* bitmap, uint16_t width, uint16_t height, uint16_t color) override {
        const int bytesPerCol = (width-1)/8+1;
        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                auto pixel = bitmap[h*bytesPerCol + w/8] & (0b10000000 >> (w%8));
                mvprintw(y+h, x+w, pixel == 0 ? " " : "*");
            }
        }
    }

    size_t drawBuffer(uint16_t x, uint16_t y, const char *buffer, size_t len) override {
        return mvprintw(y, x, buffer, len);
    }
};
