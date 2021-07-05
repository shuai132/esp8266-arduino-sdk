#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <cstdint>
#include "game_engine_port.h"

namespace ge {

using byte = unsigned char;

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;

    void operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;

    ~noncopyable() = default;
};

struct Point {
    float x = 0;
    float y = 0;
};
using Position = Point;

class Node : noncopyable {
public:
    virtual void update(float deltaMs) {
        for (const auto& item : childs) {
            item->update(deltaMs);
        }
    }

    virtual void onDraw(Canvas* canvas) {
        for (const auto& item : childs) {
            item->onDraw(canvas);
        }
        consumeFrameEndCbs();
    }

    void addChild(Node* node) {
        node->_parent = this;
        runOnFrameEnd([this, node] {
            childs.push_back(node);
        });
    }

    void removeChild(Node* node) {
        runOnFrameEnd([this, node] {
            childs.erase(std::find(childs.begin(), childs.end(), node));
        });
    }

    void removeFromParent() {
        if (_parent) {
            _parent->removeChild(this);
        }
    }

    void runOnFrameEnd(std::function<void()> cb) {
        frameEndCbs.push_back(std::move(cb));
    }

private:
    void consumeFrameEndCbs() {
        for (const auto& item : frameEndCbs) {
            item();
        }
        frameEndCbs.clear();
    }

public:
    std::vector<Node*> childs;
    Position pos;

private:
    Node* _parent = nullptr;  // weak reference
    std::vector<std::function<void()>> frameEndCbs;
};

struct Bitmap {
    uint16_t width = 0;
    uint16_t height = 0;
    const byte* data = nullptr;
};

class Spirit : public Node {
public:
    void onDraw(Canvas* canvas) override {
        canvas->drawBitmap(pos.x, pos.y, bitmap.data, bitmap.width, bitmap.height, 1);
    }

public:
    Bitmap bitmap;
};

class Scene : public Node {
public:
    virtual void onUpdate(float deltaMs) {
        canvas->onClear();
        update(deltaMs);
        onDraw(canvas);
        canvas->onDraw();
    }

public:
    Canvas* canvas;
};

class Director : noncopyable {
public:
    void start(uint16_t fps = 15) {
        _fps = fps;
        _intervalUs = 1000000 / _fps;
        _running = true;
        _lastStartTime = nowUs();
        while (_running) {
            loop();
        }
    }

    void stop() {
        _running = false;
    }

private:
    void loop() {
        auto startTime = nowUs();
        scene->onUpdate(float(startTime - _lastStartTime) / 1000.f);
        auto endTime = nowUs();
        _lastStartTime = startTime;

        auto deltaUs = endTime - startTime;
        long shouldDelayUs = _intervalUs - deltaUs;
        if (shouldDelayUs > 0) {
            delayUs(shouldDelayUs);
        }
    }

public:
    Scene* scene;

private:
    bool _running = false;
    uint16_t _fps;
    unsigned long _intervalUs;
    unsigned long _lastStartTime;
};

}
