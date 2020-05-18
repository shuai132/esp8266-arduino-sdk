#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include <algorithm>

#include "config.h"
#include "log.h"
#include "PacketProcessor.h"
#include "MsgParser.h"

#define RELAY_PIN 2

static AsyncClient* client;

static PacketProcessor packetProcessor(true);
static MsgParser msgParser;

static void sendRaw(const std::string& data) {
    client->write(data.data(), data.size());
}

static void sendRaw(const void* data, size_t size) {
    client->write((char*)data, size);
}

static void sendMsg(const std::string& type, const std::string& msg) {
    auto jsonMsg = msgParser.makeMsg(type, msg);
    auto payload = packetProcessor.pack(jsonMsg);
    sendRaw(payload.data(), payload.size());
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
    LOGD("handleData: from: %s, len: %u, data: %s"
            , client->remoteIP().toString().c_str()
            , len
            , std::string((char*)data, std::min(len, 10U)).c_str()
            );
    packetProcessor.feed((uint8_t*)data, len);
}

static void onConnect(void* arg, AsyncClient* client) {
    LOGD("onConnect: host_ip: %s host_port:%d", TCP_HOST, TCP_PORT);
    sendMsg(Msg::Type::MSG, "hello");
}

void setup() {
    Serial.begin(115200);
    delay(20);

    client = new AsyncClient;
    client->onData(&handleData, client);
    client->onConnect(&onConnect, client);
    client->onDisconnect([](void*, AsyncClient*){
        LOGD("client disconnect");
    }, client);

    // user logic
    pinMode(RELAY_PIN, OUTPUT);

    packetProcessor.setMaxBufferSize(1024);
    packetProcessor.setOnPacketHandle([](uint8_t* data, size_t size) {
        msgParser.parser(std::string((char*)data, size));//todo: performance
    });

    msgParser.setRelayCb([](bool relay, MsgParser::ID_t id) {
        LOGD("relay set to: %d", !relay);
        digitalWrite(RELAY_PIN, !relay);
        sendRaw(msgParser.makeRsp(id));
    });

    msgParser.setHostRegexCb([](std::string hostRegex) {
        LOGD("HostRegexCb: %s", hostRegex.c_str());
    });
    msgParser.setHostPasswdCb([](std::string passwd) {
        LOGD("HostPasswdCb: %s", passwd.c_str());
    });
}

void loop() {
    if (not WiFi.isConnected()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, PASSWORD);
        while (WiFi.status() != WL_CONNECTED) {
            LOGD("WiFi connecting...");
            delay(500);
        }
    }

    if (not client->connected()) {
        client->connect(TCP_HOST, TCP_PORT);
        delay(500);
    }
}
