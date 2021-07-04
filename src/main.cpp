extern "C" {
#include <osapi.h>
#include <os_type.h>
}

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <EEPROM.h>

#include "config.h"
#include "PacketProcessor.h"
#include "GPIO.h"
#include "WifiScan.h"
#include "log.h"
#include "RpcCore.hpp"
#include "SimpleTimer/SimpleTimer.h"
#include "AppMsg.h"

#ifdef USE_OLED
#include "OLED.h"
#undef  PIN_RELAY
#define PIN_RELAY                   -1
#undef  LOGI
#define LOGI(fmt, ...)              OLED_printf(fmt, ##__VA_ARGS__)
#else
#define OLED_Init()                 ((void)0)
#endif

static gpio::OUT relay(PIN_RELAY, LOW);
static SimpleTimer timer;

static AsyncClient* client;
static WiFiScan wiFiScan;

static PacketProcessor packetProcessor(true);
static std::shared_ptr<RpcCore::Connection> conn;
static std::shared_ptr<RpcCore::Rpc> rpc;

const size_t MAX_SSIDRE_LEN = 50 + 1;
const size_t MAX_PASSWD_LEN = 16 + 1;
const uint16_t HOST_INFO_CONFIGED = 0x5AA5;
struct HostInfo {
    uint16_t configed;
    char ssidRE[MAX_SSIDRE_LEN];
    char passwd[MAX_PASSWD_LEN];
    uint8_t boardId;
};

static HostInfo* hostInfo;

static void initTcpClient() {
    client = new AsyncClient;
    client->onData([](void* arg, AsyncClient* client, void *data, size_t len) {
        LOGD("handleData: from: %s, len: %u, data: %s"
        , client->remoteIP().toString().c_str()
        , len
        , std::string((char*)data, std::min(len, 10U)).c_str()
        );
        packetProcessor.feed(data, len);
    }, client);
    client->onConnect([](void* arg, AsyncClient* client) {
        LOGI("onConnect: ip: %s", WiFi.gatewayIP().toString().c_str());
    }, client);
    client->onDisconnect([](void*, AsyncClient*){
        LOGI("onDisconnect: %s", WiFi.gatewayIP().toString().c_str());
    }, client);
}

static void iniRpc() {
    using namespace RpcCore;
    using String = RpcCore::String;

    // 初始化
    conn = std::make_shared<Connection>([](const std::string& payload) {
        packetProcessor.packForeach(payload.data(), payload.size(),
                                    [](uint8_t* data, size_t size) {
                                        client->write((char*)data, size);
                                    });
    });
    rpc = std::make_shared<Rpc>(conn);
    rpc->setTimerImpl([](uint32_t ms, Rpc::TimeoutCb cb) {
        timer.setTimeout(ms, std::move(cb));
    });

    rpc->subscribe<Raw<bool>>("setRelay", [](Raw<bool> state) {
        LOGI("setRelay: %d", state.value);
        relay.set(state.value);
    });
    rpc->subscribe<Raw<bool>>("getRelay", []() {
        auto val = relay.value(true);
        LOGI("getRelay: %d", val);
        return val;
    });
    // 设置继电器维持某个电平多久
    using Action = RpcCore::Struct<RelayAction>;
    rpc->subscribe<Action>("createRelayAction", [](const Action& msg) {
        const auto& action = msg.value;
        LOGI("createRelayAction: start: %d, ms:%d, end:%d", action.valStart, action.delayMs, action.valEnd);
        relay.set(action.valStart);
        timer.setTimeout(action.delayMs, [action]{
            if (relay.value(true) == action.valEnd) return;
            relay.set(action.valEnd);
        });
    });

    rpc->subscribe<String, Raw<bool>>("setHostRege", [](const String& hostRegex) {
        LOGI("setHostRege: %s", hostRegex.c_str());
        if (hostRegex.length() > MAX_SSIDRE_LEN - 1) {
            LOGD("hostRegex too long");
            return false;
        } else {
            wiFiScan.setSSIDEnds(hostRegex);
            strcpy(hostInfo->ssidRE, hostRegex.c_str());
            EEPROM.commit();
            return true;
        }
    });

    rpc->subscribe<String, Raw<bool>>("setHostPasswd", [](const String& passwd) {
        LOGI("setHostPasswd: %s", passwd.c_str());
        if (passwd.length() > MAX_SSIDRE_LEN - 1) {
            LOGD("passwd too long");
            return false;
        } else {
            strcpy(hostInfo->passwd, passwd.c_str());
            EEPROM.commit();
            return true;
        }
    });
    rpc->subscribe<String>("getHostRege", [] {
        LOGD("getHostRege: %s", hostInfo->ssidRE);
        return hostInfo->ssidRE;
    });
    rpc->subscribe<String>("getHostPasswd", [] {
        LOGD("getHostPasswd: %s", hostInfo->passwd);
        return hostInfo->passwd;
    });

    using Info = RpcCore::Struct<DeviceInfo>;
    rpc->subscribe<Info>("setDeviceInfo", [](const Info& info) {
        hostInfo->boardId = info.value.boardId;
        EEPROM.commit();
    });
    rpc->subscribe<Info>("getDeviceInfo", [] {
        return DeviceInfo{system_get_chip_id(), hostInfo->boardId};
    });
    rpc->subscribe<Raw<int8_t>>("getRssi", [] {
        return Raw<int8_t>{wifi_station_get_rssi()};
    });
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
        LOGD("init hostInfo to EEPROM: ssidRE: %s, passwd: %s", hostInfo->ssidRE, hostInfo->passwd);
    } else {
        LOGD("use hostInfo from EEPROM: ssidRE: %s, passwd: %s", hostInfo->ssidRE, hostInfo->passwd);
    }
#else
    strcpy(hostInfo->ssidRE, SSID_RE_DEFAULT);
    strcpy(hostInfo->passwd, PASSWORD_DEFAULT);
    LOGD("use default hostInfo: ssidRE: %s, passwd: %s", hostInfo->ssidRE, hostInfo->passwd);
#endif
}

#include "game/chrome_game.h"
//#include "test.h"

void setup() {
//    test();
    start_game();

    Serial.begin(115200);
    delay(20);
    std::set_new_handler([] {
        FATAL("out of memory");
    });

    LOGI("Hello World");

    initHostFromEEPROM();

    LOGD("init wiFiScan");
    wiFiScan.setSSIDEnds(hostInfo->ssidRE);

    LOGD("init packetProcessor");
    packetProcessor.setMaxBufferSize(1024);
    packetProcessor.setOnPacketHandle([](uint8_t* data, size_t size) {
        conn->onRecvPacket(std::string((char*)data, size));
    });

    LOGD("init Rpc");
    iniRpc();

    LOGD("init Client");
    initTcpClient();
}

void loop() {
    timer.run();

    if (not WiFi.isConnected()) {
        WiFi.mode(WIFI_STA);

#ifdef TEST_WITH_DESKTOP
        auto ssid = wiFiScan.scan();
        if (ssid.empty()) {
            LOGD("scan empty");
            delay(1000);
        }

        LOGD("try connect to: %s", ssid.c_str());

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
#ifdef TEST_WITH_DESKTOP
        client->connect(TEST_WITH_DESKTOP_IP, TEST_WITH_DESKTOP_PORT);
#else
        client->connect(WiFi.gatewayIP(), TCP_PORT);
#endif
        delay(500);
    }
}
