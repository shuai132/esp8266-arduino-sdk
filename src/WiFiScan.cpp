#include <ESP8266WiFi.h>

#include "WifiScan.h"
#include "config.h"
#include "log.h"

void WiFiScan::setSSIDEnds(std::string ssid) {
    _ssidEnds = std::move(ssid);
}

/**
 * @return wifi ssid  扫描名称符合条件的AP
 * 1. 为空时表示未扫描到
 * 2. 可能返回系统配置AP
 */
std::string WiFiScan::scan() {
    auto networks = WiFi.scanNetworks();
    LOGD("networks: %d", networks);
    if (networks < 0) {
        LOGE("scan failed");
        return "";
    }

    for (uint8_t item = 0; item < networks; item++) {
        String ssid;
        uint8_t encryptionType;
        int32_t RSSI;
        uint8_t* BSSID;
        int32_t channel;
        bool isHidden;

        if(not WiFi.getNetworkInfo(item, ssid, encryptionType, RSSI, BSSID, channel, isHidden)) {
            LOGD("getNetworkInfo error: item: %d", item);
            continue;
        }

        LOGD("item: %d, ssid: %s, RSSI: %d", item, ssid.c_str(), RSSI);
        std::string ssidStr(ssid.c_str());
        if (ssid.endsWith(_ssidEnds.c_str())) {
            LOGD("find match ap: %s", ssidStr.c_str());
            onMatchAP(ssidStr);
            return ssidStr;
        }

        if (ssid == CONFIG_AP_SSID) {
            return ssidStr;
        }
    }
    return "";
}

