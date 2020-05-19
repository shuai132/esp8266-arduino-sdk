#include <algorithm>

extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <EEPROM.h>

#include "config.h"
#include "log.h"
#include "PacketProcessor.h"
#include "MsgParser.h"
#include "Relay.h"
#include "WifiScan.h"

static AsyncClient* client;

static PacketProcessor packetProcessor(true);
static MsgParser msgParser;

static Relay* relay;

static WiFiScan wiFiScan;

const size_t MAX_SSIDRE_LEN = 50 + 1;
const size_t MAX_PASSWD_LEN = 16 + 1;
const uint16_t HOST_INFO_CONFIGED = 0x5AA5;
struct HostInfo {
    uint16_t configed;
    char ssidRE[MAX_SSIDRE_LEN];
    char passwd[MAX_PASSWD_LEN];
};

static HostInfo* hostInfo;

static void sendRaw(const std::string& data) {
    client->write(data.data(), data.size());
}

static void sendRaw(const void* data, size_t size) {
    client->write((char*)data, size);
}

static void sendJasonMsg(const std::string& jsonMsg) {
    auto payload = packetProcessor.pack(jsonMsg);
    sendRaw(payload.data(), payload.size());
}

static void sendMsgByTemplate(const std::string& type, const std::string& msg) {
    auto jsonMsg = msgParser.makeMsg(type, msg);
    sendJasonMsg(jsonMsg);
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
    LOGD("onConnect: host_ip: %s host_port:%d", WiFi.gatewayIP().toString().c_str(), TCP_PORT);
    sendMsgByTemplate(Msg::Type::MSG, "hello");
}

static void initHostFromEEPROM() {
    EEPROM.begin(sizeof(HostInfo));
    hostInfo = reinterpret_cast<HostInfo*>(EEPROM.getDataPtr());

#if TRY_USE_EEPROM_INFO
    if (hostInfo->configed != HOST_INFO_CONFIGED) {
        hostInfo->configed = HOST_INFO_CONFIGED;
        strcpy(hostInfo->ssidRE, SSID_RE_DEFAULT);
        strcpy(hostInfo->passwd, PASSWORD_DEFAULT);
        EEPROM.commit();
    } else {
        LOGD("use hostInfo from EEPROM: ssidRE: %s, passwd: %s", hostInfo->ssidRE, hostInfo->passwd);
    }
#else
    strcpy(hostInfo->ssidRE, SSID_RE_DEFAULT);
    strcpy(hostInfo->passwd, PASSWORD_DEFAULT);
    LOGD("use default hostInfo: ssidRE: %s, passwd: %s", hostInfo->ssidRE, hostInfo->passwd);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(20);

    initHostFromEEPROM();

    LOGD("init wiFiScan");
    wiFiScan.setSSIDEnds(hostInfo->ssidRE);

    LOGD("init relay");
    relay = new Relay(RELAY_PIN);

    LOGD("init client");
    client = new AsyncClient;
    client->onData(&handleData, client);
    client->onConnect(&onConnect, client);
    client->onDisconnect([](void*, AsyncClient*){
        LOGD("client disconnect");
    }, client);

    LOGD("init packetProcessor");
    packetProcessor.setMaxBufferSize(1024);
    packetProcessor.setOnPacketHandle([](uint8_t* data, size_t size) {
        msgParser.parser(std::string((char*)data, size));//todo: performance
    });

    LOGD("init msgParser");
    msgParser.setRelayCb([](bool on, MsgParser::ID_t id) {
        LOGD("relay pin set to: %d", !on);
        relay->set(!on);

        sendJasonMsg(msgParser.makeRsp(id));
    });

    LOGD("init message callback");
    msgParser.setHostRegexCb([](const std::string& hostRegex, MsgParser::ID_t id) {
        LOGD("HostRegexCb: %s", hostRegex.c_str());
        wiFiScan.setSSIDEnds(hostRegex);
        auto str = hostRegex.c_str();
        // 限定保存长度 注意最后一位\0
        memcpy(hostInfo->ssidRE, str, std::min(strlen(str) + 1, MAX_SSIDRE_LEN));
        hostInfo->ssidRE[MAX_SSIDRE_LEN - 1] = '\0';
        EEPROM.commit();

        sendJasonMsg(msgParser.makeRsp(id));
    });
    msgParser.setHostPasswdCb([](const std::string& passwd, MsgParser::ID_t id) {
        LOGD("HostPasswdCb: %s", passwd.c_str());
        auto str = passwd.c_str();
        memcpy(hostInfo->passwd, str, std::min(strlen(str) + 1, MAX_PASSWD_LEN));
        hostInfo->passwd[MAX_PASSWD_LEN - 1] = '\0';
        EEPROM.commit();

        sendJasonMsg(msgParser.makeRsp(id));
    });
}

void loop() {
    if (not WiFi.isConnected()) {
        WiFi.mode(WIFI_STA);

#if TEST_WITH_DESKTOP
        WiFi.begin(TEST_WITH_DESKTOP_SSID, TEST_WITH_DESKTOP_PASSWD);
#else
        SCAN:
        auto ssid = wiFiScan.scan();
        if (ssid.empty()) {
            LOGD("scan empty");
            delay(1000);
            goto SCAN;
        }

        LOGD("try connect to: %s", ssid.c_str());

        auto isConfigAp = ssid == CONFIG_AP_SSID;
        WiFi.begin(ssid.c_str(), isConfigAp ? CONFIG_AP_PASSWD : hostInfo->passwd);
#endif
        while (WiFi.status() != WL_CONNECTED) {
            LOGD("WiFi connecting...");
            delay(500);
        }
        LOGD("gatewayIP: %s", WiFi.gatewayIP().toString().c_str());
    }

    if (not client->connected()) {
#if TEST_WITH_DESKTOP
        client->connect(TEST_WITH_DESKTOP_IP, TEST_WITH_DESKTOP_PORT);
#else
        client->connect(WiFi.gatewayIP(), TCP_PORT);
#endif
        delay(500);
    }
}
