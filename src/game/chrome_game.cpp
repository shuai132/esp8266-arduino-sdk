#include "chrome_game.h"
#include "game_engine.hpp"
#include "screen/OLEDScreen.h"

class Dragon : public Spirit {
public:
    Dragon() {
        bitmap.width = 3;
        bitmap.height = 3;
        bitmap.data = reinterpret_cast<byte*>(bitmapDragon);
        pos.x = 10;
        pos.y = 20;
    }
    void update(FrameInfo info) override {
        Spirit::update(info);
        if (--pos.y < 5) {
            pos.y = 20;
        }
    }

    void onDraw(Canvas *fb) override {
        fb->drawCircle(pos.x, pos.y, 5, 1);
    }

    byte bitmapDragon[3][3] = {
            {0, 1, 0},
            {1, 1, 1},
            {0, 1, 0},
    };
};

class Tree : public Spirit {
public:
    Tree() {
        bitmap.width = 2;
        bitmap.height = 3;
        bitmap.data = reinterpret_cast<byte*>(bitmapTree);
    }
    void update(FrameInfo info) override {
        Spirit::update(info);
        pos.x--;
        if (pos.x + bitmap.width < 0) {
            pos.x = info.fb->width() - bitmap.width;
        }
    }

    void onDraw(Canvas *fb) override {
        fb->drawRect(pos.x, fb->height() - 13, 6, 13, 1);
    }

    byte bitmapTree[3][2] = {
            {1, 1},
            {1, 1},
            {1, 1},
    };
};

class GameScene : public Scene {
public:
    GameScene() {
        dragon = std::make_shared<Dragon>();
        tree1 = std::make_shared<Tree>();
        tree2 = std::make_shared<Tree>();
        tree3 = std::make_shared<Tree>();
        tree4 = std::make_shared<Tree>();
        addChild(dragon);
        addChild(tree1);
        addChild(tree2);
        addChild(tree3);
        addChild(tree4);

        tree1->pos.x = 0;
        tree2->pos.x = 32*1;
        tree3->pos.x = 32*2;
        tree4->pos.x = 32*3;
    }

    std::shared_ptr<Dragon> dragon;
    std::shared_ptr<Tree> tree1;
    std::shared_ptr<Tree> tree2;
    std::shared_ptr<Tree> tree3;
    std::shared_ptr<Tree> tree4;
};

void game_task() {
    auto game = std::make_shared<Director>();
    game->scene = std::make_shared<GameScene>();
    game->scene->screen = std::make_shared<OLEDScreen>();
    game->start(10);
}
