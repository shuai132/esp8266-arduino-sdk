#include "chrome_game.h"

#include <string>
#include <cmath>
#include "game_engine.hpp"
#include "img/dragon.h"
#include "img/tree.h"

#ifdef ARDUINO
#include "OLED.h"
#include "impl/game_engine_port_arduino.h"
#else
#include "impl/game_engine_port_desktop.h"
#endif

using namespace ge;

class Dragon : public Spirit {
public:
    Dragon() {
        bitmap.width = dragon_width;
        bitmap.height = dragon_height;
        bitmap.data = dragon_data;
        reset();
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
        _speed = speedInit;
    }
    void reset() {
        pos.x = 10;
        pos.y = SCREEN_HEIGHT - dragon_height;
    }

public:
    bool isOnGround() const { return _onGround; }

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
        canvas->drawText(SCREEN_WIDTH - PIX_PER_CHAR*5, 0, "%05d", (int)score);
        canvas->drawText(SCREEN_WIDTH - PIX_PER_CHAR*(5+7), 0, "FPS:%d", (int)ceil(realFps));
    }
    void update(float deltaMs) override {
        realFps = 1000 / deltaMs;
    }
    float score = 0;
    float realFps = 0;
};

class GameLogic : public Node {
public:
    explicit GameLogic(Dragon* dragon, std::vector<Tree*>* trees)
        : _dragon(dragon), _trees(trees) {}

    void update(float deltaMs) override {
        Node::update(deltaMs);
        for (const auto& tree : *_trees) {
            if (abs(_dragon->pos.x - tree->pos.x) < dragon_width * 0.2f) {
                // 贴近的时候判断高度 要大于一定数值
                if (_dragon->pos.y + dragon_height > tree->pos.y + tree2_height * 0.2f) {
                    onGameOver();
                } else {
                    // will trigger more times
                    //onPassTree();
                }
            }
        }
    }
    void onDraw(Canvas *canvas) override {
        if (isGameOver) {
            const std::string gameOver("GAME OVER");
            canvas->drawString((SCREEN_WIDTH - PIX_PER_CHAR*(gameOver.length())) / 2, SCREEN_HEIGHT / 2, gameOver.c_str());
        }
    }

private:
    void onGameOver() {
        isGameOver = true;
        if (gameOverCb) gameOverCb();
    }
    void onPassTree() {
        if (passTreeCb) passTreeCb();
    }

public:
    std::function<void()> gameOverCb;
    std::function<void()> passTreeCb;
    bool isGameOver = false;

private:
    Dragon* _dragon;
    std::vector<Tree*>* _trees;
};

class GameScene : public Scene {
public:
    GameScene() {
        addChild(_dragon);
        addChild(_score);
        addChild(_gameLogic);

        for (int i = 0; i < treeNum; ++i) {
            auto tree = new Tree;
            addChild(tree);
            _trees.push_back(tree);
        }

        _gameLogic->passTreeCb = [this]{
            _score->score += 100;
        };

        restart();
    }

protected:
    void update(float deltaMs) override {
        if (_gameLogic->isGameOver) {
            if (checkButton()) {
                restart();
            }
            return;
        }
        Scene::update(deltaMs);
        _score->score += deltaMs / 100; // one score per 100ms

        if (checkButton()) {
            _dragon->tap();
        }
    };

private:
    void restart() {
        _gameLogic->isGameOver = false;
        _score->score = 0;
        _dragon->reset();
        for (int i = 0; i < treeNum; ++i) {
            const auto& tree = _trees[i];
            tree->pos.x = SCREEN_WIDTH / treeNum * (i + 1);
            tree->pos.y = SCREEN_HEIGHT - tree->bitmap.height;
        }
    }

private:
    const int treeNum = 2;
    Dragon* _dragon = new Dragon;
    Score* _score = new Score;
    std::vector<Tree*> _trees;
    GameLogic* _gameLogic = new GameLogic(_dragon, &_trees);
};

void start_game() {
    auto game = new Director();
    game->scene = new GameScene();
    game->scene->canvas = new Screen();
    game->start(120);
}
