#pragma once

#include <cstdint>

#pragma pack(push, 4)

struct RelayAction {
    uint8_t valStart;
    uint32_t delayMs;
    uint8_t valEnd;
};

#pragma pack(pop)
