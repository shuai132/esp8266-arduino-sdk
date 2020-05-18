#include <ESP8266WiFi.h>

#include "WifiScan.h"
#include "log.h"

void WiFiScan::setSSIDEnds(std::string ssid) {
    _ssidEnds = std::move(ssid);
}

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
    }
    return "";
}

