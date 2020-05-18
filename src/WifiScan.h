#pragma once

#include <string>
#include <utility>

#include "noncopyable.h"
#include "MakeEvent.hpp"

class WiFiScan : noncopyable {
public:
    MAKE_EVENT(MatchAP, std::string ssid);

public:
    void setSSIDEnds(std::string ssid);
    std::string scan();

private:
    std::string _ssidEnds;
};
