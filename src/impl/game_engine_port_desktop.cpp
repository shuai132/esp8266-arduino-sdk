#ifndef ARDUINO
#include "game_engine_port_desktop.h"
#include <thread>

namespace ge {

using namespace std::chrono;

unsigned long nowMs() {
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
unsigned long nowUs() {
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
void delayUs(unsigned int us) {
    std::this_thread::sleep_for(microseconds(us));
}

void delayMs(unsigned int us) {
    std::this_thread::sleep_for(milliseconds(us));
}

bool checkButton() {
    static volatile bool isClick = false;
    static auto t = std::thread([]{
        for(;;) {
            isClick = false;
            auto ch = getch();
            if (ch == ' ') {
                isClick = true;
            } else if (ch == 'q') {
                exit(0);
            }
            delayMs(300);
        }
    });
    return isClick;
}

}

#endif
