#include "chrome_game.h"
#include "game_engine.hpp"
#include "screen/OLEDScreen.h"
#include "img/dragon.h"
#include "img/tree.h"

class Dragon : public Spirit {
public:
    Dragon() {
        bitmap.width = dragon_width;
        bitmap.height = dragon_height;
        bitmap.data = dragon_data;
        pos.x = 10;
        pos.y = SCREEN_HEIGHT - dragon_height;
    }
    void update(FrameInfo info) override {
        Spirit::update(info);
        // update step
        if (bitmap.data == dragon_data_1) {
            bitmap.data = dragon_data_2;
        } else {
            bitmap.data = dragon_data_1;
        }
        auto shouldMovePix = [&]()->int{
            return 0;
            auto pix = info.deltaMs*speed/1000;
            return pix > 1 ? pix : 1;
        };
        if (up) {
            pos.y -= shouldMovePix();
            if (pos.y < 0) {
                up = false;
            }
        }
        if (!up) {
            pos.y += shouldMovePix();
            if (pos.y > SCREEN_HEIGHT - bitmap.height) {
                up = true;//todo
            }
        }
    }

private:
    bool up = false;
    const int speed = 50; // 每秒像素点
};

class Tree : public Spirit {
public:
    Tree() {
        bitmap.width = tree1_width;
        bitmap.height = tree1_height;
        bitmap.data = tree1_data;
    }
    void update(FrameInfo info) override {
        Spirit::update(info);
        pos.x--;
        if (pos.x + bitmap.width < 0) {
            pos.x = info.canvas->width() - bitmap.width;
        }
    }
};

class GameScene : public Scene {
public:
    GameScene() {
        addChild(std::make_shared<Dragon>());
        const int treeNum = 3;
        for (int i = 0; i < treeNum; ++i) {
            auto tree = std::make_shared<Tree>();
            tree->pos.x = SCREEN_WIDTH / treeNum * (i+1);
            tree->pos.y = SCREEN_HEIGHT - tree->bitmap.height;
            addChild(tree);
        }
    }
};

void game_task() {
    auto game = std::make_shared<Director>();
    game->scene = std::make_shared<GameScene>();
    game->scene->screen = std::make_shared<OLEDScreen>();
    game->start(15);
}
