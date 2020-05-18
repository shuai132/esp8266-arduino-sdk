#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include <algorithm>

#include "config.h"
#include "log.h"

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
    LOGD("handleData: from: %s, len: %u, data: %s"
            , client->remoteIP().toString().c_str()
            , len
            , std::string((char*)data, std::min(len, 10U)).c_str()
            );
    // echo
    client->write((char*)data, len);
}

static void onConnect(void* arg, AsyncClient* client) {
    LOGD("onConnect: host_ip: %s host_port:%d \n", TCP_HOST, TCP_PORT);
    client->write("hello");
}

static AsyncClient* client;

void setup() {
    Serial.begin(115200);
    delay(20);

    client = new AsyncClient;
    client->onData(&handleData, client);
    client->onConnect(&onConnect, client);
    client->onDisconnect([](void*, AsyncClient*){
        LOGD("client disconnect");
    }, client);
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
