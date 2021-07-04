#pragma once

#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <Arduino.h>
#include "noncopyable.h"

#include "OLED.h"

using byte = unsigned char;

struct Point {
    int16_t x;
    int16_t y;
};
using Position = Point;

struct FrameInfo {
    float deltaMs;
    Canvas* canvas;
};

class Node : noncopyable {
public:
    virtual void update(FrameInfo info) {
        for (const auto& item : childs) {
            item->update(info);
        }
        consumeFrameEndCbs();
    }

    void addChild(std::shared_ptr<Node> node) {
        node->_parent = this;
        runOnFrameEnd([this, node = std::move(node)]{
            childs.push_back(node);
        });
    }

    void removeChild(std::shared_ptr<Node> node) {
        runOnFrameEnd([this, node = std::move(node)]{
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
    std::vector<std::shared_ptr<Node>> childs;
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
    void update(FrameInfo info) override {
        onDraw(info.fb);
        Node::update(info);
    }
    virtual void onDraw(Canvas* canvas) {
        canvas->drawBitmap(pos.x, pos.y, bitmap.data, bitmap.width, bitmap.height, 1);
    }

public:
    Bitmap bitmap;
};

struct Screen : noncopyable {
    virtual void onClear() = 0;
    virtual void onDraw() = 0;
    virtual Canvas* getCanvas() = 0;
};

class Scene : public Node {
public:
    void doUpdate(float deltaMs) {
        assert(screen);
        screen->onClear();
        onUpdate(deltaMs);
        screen->onDraw();
    }

protected:
    virtual void onUpdate(float deltaMs) {
        Node::update({deltaMs, screen->getCanvas()});
    }

public:
    std::shared_ptr<Screen> screen;
};

class Director : noncopyable {
public:
    void start(uint16_t fps = 15) {
        _fps = fps;
        _intervalUs = 1000000.f / _fps;
        _running = true;
        _lastStartTime = micros();
        while(_running){
            loop();
        }
    }
    void stop() {
        _running = false;
    }

private:
    void loop() {
        auto startTime = micros();
        scene->doUpdate(float(1) / 1000.f);
        auto endTime = micros();
        _lastStartTime = endTime;

        auto deltaUs = endTime - startTime;
        int64_t shouldDelayUs = _intervalUs - deltaUs;
        if (shouldDelayUs > 0) {
            delayMicroseconds(shouldDelayUs);
        }
    }

public:
    std::shared_ptr<Scene> scene;

private:
    bool _running = false;
    uint16_t _fps{};
    uint32_t _intervalUs{};

    unsigned long _lastStartTime;
};
