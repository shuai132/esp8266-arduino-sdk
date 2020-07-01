#pragma once

#include <cstdint>

#pragma pack(push, 4)

struct RelayAction {
    uint8_t volFrom;
    uint32_t delayMs;
    uint8_t volTo;
};

struct DeviceInfo {
    uint32_t chipId;
    uint8_t boardId;
};

#pragma pack(pop)
