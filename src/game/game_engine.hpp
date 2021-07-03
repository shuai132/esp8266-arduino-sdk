#pragma once

#include <vector>
#include <thread>
#include <chrono>
#include <cstdint>
#include <functional>
#include <algorithm>
#include "noncopyable.h"

using byte = unsigned char;

struct Point {
    int16_t x;
    int16_t y;
};
using Position = Point;

struct FrameBuffer : noncopyable {
    const uint16_t width;
    const uint16_t height;
    std::vector<byte> buffer;

    explicit FrameBuffer(uint16_t width = 32, uint16_t height = 8)
            : width(width), height(height) {
        clear();
    }

    void clear() {
        buffer.clear();
        buffer.resize(width * height);
    }
};

struct FrameInfo {
    float deltaMs;
    FrameBuffer* fb;
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
//            childs.erase(std::find(childs.cbegin(), childs.cend(), node));
        });
    }

    void removeChild(Node* node) {
        runOnFrameEnd([this, node]{
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
    byte* data = nullptr;
};

class Spirit : public Node {
public:
    void update(FrameInfo info) override {
        onDraw(info.fb);
        Node::update(info);
    }
    virtual void onDraw(FrameBuffer* fb) {
        for(int h = 0; h < bitmap.height; h++) {
            for(int w = 0; w < bitmap.width; w++) {
                if (h+pos.y >= fb->height || w+pos.x >= fb->width) continue;
                fb->buffer[(h+pos.y)*fb->width + (w+pos.x)] = bitmap.data[h*bitmap.width + w];
            }
        }
    }

public:
    Bitmap bitmap;
};

struct Screen : noncopyable {
    virtual void onDraw(FrameBuffer* frameBuffer) = 0;
};

class Scene : public Node {
public:
    void update(float deltaMs) {
        _frameBuffer.clear();
        onUpdate(deltaMs);
        if (screen) {
            screen->onDraw(&_frameBuffer);
        }
    }

protected:
    virtual void onUpdate(float deltaMs) {
        Node::update({deltaMs, &_frameBuffer});
    }

public:
    std::shared_ptr<Screen> screen;

private:
    FrameBuffer _frameBuffer;
};

#include "Arduino.h"

class Director : noncopyable {
public:
    void start(uint16_t fps = 15) {
        _fps = fps;
        _intervalUs = 1000000.f / _fps;
        _running = true;
        _lastStartTime = std::chrono::steady_clock::now();
        while(_running){
            loop();
        }
    }
    void stop() {
        _running = false;
    }

private:
    void loop() {
        printf("loop\n");
        auto now = std::chrono::steady_clock::now;

        auto startTime = now();
        scene->update(float(1) / 1000.f);
        auto endTime = now();
        delay(1000);
    }

public:
    std::shared_ptr<Scene> scene;

private:
    bool _running = false;
    uint16_t _fps{};
    uint32_t _intervalUs{};

    std::chrono::steady_clock::time_point _lastStartTime;
};
