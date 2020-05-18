#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include "config.h"
#include "log.h"
#include "PacketProcessor.h"
#include "MsgParser.h"
#include "Relay.h"

static AsyncClient* client;

static PacketProcessor packetProcessor(true);
static MsgParser msgParser;

static Relay* relay;

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

    relay = new Relay(RELAY_PIN);

    client = new AsyncClient;
    client->onData(&handleData, client);
    client->onConnect(&onConnect, client);
    client->onDisconnect([](void*, AsyncClient*){
        LOGD("client disconnect");
    }, client);

    packetProcessor.setMaxBufferSize(1024);
    packetProcessor.setOnPacketHandle([](uint8_t* data, size_t size) {
        msgParser.parser(std::string((char*)data, size));//todo: performance
    });

    msgParser.setRelayCb([](bool on, MsgParser::ID_t id) {
        LOGD("relay pin set to: %d", !on);
        relay->set(!on);
        sendRaw(msgParser.makeRsp(id));
    });

    msgParser.setHostRegexCb([](const std::string& hostRegex) {
        LOGD("HostRegexCb: %s", hostRegex.c_str());
    });
    msgParser.setHostPasswdCb([](const std::string& passwd) {
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
