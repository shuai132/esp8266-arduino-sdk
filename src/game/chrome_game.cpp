#include "chrome_game.h"
#include "game_engine.hpp"
#include "img/dragon.h"
#include "img/tree.h"
#include "OLED.h"
#include "game/impl/game_engine_port_arduino.h"

using namespace ge;

class Dragon : public Spirit {
public:
    Dragon() {
        bitmap.width = dragon_width;
        bitmap.height = dragon_height;
        bitmap.data = dragon_data;
        pos.x = 10;
        pos.y = SCREEN_HEIGHT - dragon_height;
    }
    void update(float deltaMs) override {
        Spirit::update(deltaMs);
        updateBitmap();
        if (_onGround) return;
        auto shouldMovePix = _speed * deltaMs;
        _speed -= deltaMs * acceleration;
        pos.y -= shouldMovePix;
        if (pos.y >= SCREEN_HEIGHT - bitmap.height) {
            pos.y = SCREEN_HEIGHT - bitmap.height;
            _onGround = true;
        }
    }

    void tap() {
        if (!_onGround) return;
        _onGround = false;
        _upping = true;
        _speed = speedInit;
    }

public:
    bool isOnGround() { return _onGround; }

private:
    void updateBitmap() {
        if (isOnGround()) {
            auto now = nowMs();
            if (now - _lastSwitchTime < 100) return;
            _lastSwitchTime = now;
            if (bitmap.data == dragon_data_1) {
                bitmap.data = dragon_data_2;
            } else {
                bitmap.data = dragon_data_1;
            }
        } else {
            bitmap.data = dragon_data;
        }
    }

private:
    // 跳跃的物理模拟参数 时间和高度尺度变换
    // H = 1/2 * a * t^2
    const int H = 20;           // 最高上升到像素/pix
    const float expectMs = 500; // 跳跃过程持续时间/ms
    const float halfMs = expectMs / 2;
    const double acceleration = H * 2 / (halfMs * halfMs);
    const float speedInit = acceleration * halfMs;

    bool _onGround = true;
    bool _upping = false;
    float _speed;
    unsigned long _lastSwitchTime;
};

class Tree : public Spirit {
public:
    Tree() {
        bitmap.width = tree2_width;
        bitmap.height = tree2_height;
        bitmap.data = tree2_data;
    }
    void update(float deltaMs) override {
        Spirit::update(deltaMs);
        pos.x -= _speed * deltaMs / 1000;
        if (pos.x + bitmap.width < 0) {
            pos.x = SCREEN_WIDTH - bitmap.width;
        }
    }

private:
    int _speed = 50; // pix per second
};

class Score : public Node {
public:
    void onDraw(Canvas *canvas) override {
        canvas->drawText(SCREEN_WIDTH - 6*5, 0, "%05d", (int)score);
        canvas->drawText(SCREEN_WIDTH - 6*(5+7), 0, "FPS:%d", (int)ceil(realFps));
    }
    void update(float deltaMs) override {
        realFps = 1000 / deltaMs;
    }
    float score = 0;
    float realFps = 0;
};

class GameScene : public Scene {
public:
    GameScene() {
        addChild(_dragon);
        addChild(_score);
        const int treeNum = 2;
        for (int i = 0; i < treeNum; ++i) {
            auto tree = new Tree;
            tree->pos.x = SCREEN_WIDTH / treeNum * (i + 1);
            tree->pos.y = SCREEN_HEIGHT - tree->bitmap.height;
            addChild(tree);
        }
    }

protected:
    void update(float deltaMs) override {
        ESP.wdtFeed();
        Scene::update(deltaMs);
        _score->score += deltaMs / 100; // one score per 100ms

        if (!digitalRead(0)) {
            _dragon->tap();
        }
    };

private:
    Dragon* _dragon = new Dragon;
    Score* _score = new Score;
};

void start_game() {
    auto game = new Director();
    game->scene = new GameScene();
    game->scene->canvas = new OLEDScreen();
    // for best display
    game->start(60);
}
