#pragma once

#include <ncurses.h>
#include "../game_engine.hpp"

struct NcursesScreen : public Screen {
    NcursesScreen() {
        initscr();
        raw();
        noecho();
        curs_set(0);
    }
    void onDraw(FrameBuffer* frameBuffer) override {
        auto& fb = *frameBuffer;
        auto& height = fb.height;
        auto& width = fb.width;
        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                auto pixel = fb.buffer[h*width + w];
                mvprintw(h, w, pixel == 0 ? "  " : "* ");
            }
        }
        refresh();
    }
};
