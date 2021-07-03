#pragma once

#include "../game_engine.hpp"

struct ConsoleScreen : public Screen {
    void onDraw(FrameBuffer* frameBuffer) override {
        printf("| --------------------- ConsoleScreen ----------------------- |\n");
        auto& fb = *frameBuffer;
        auto& height = fb.height;
        auto& width = fb.width;
        for(int h = 0; h < height; h++) {
            for(int w = 0; w < width; w++) {
                auto pixel = fb.buffer[h*width + w];
                printf(pixel == 0 ? "  " : "* ");
            }
            printf("\n");
        }
    }
};
