#pragma once

#include <string>
#include <variant>

enum class DeviceStatus {
    SUCCESS,
    FAILURE,
    AWAITING_CONFIRMATION
};


struct DeviceResult {
    DeviceStatus status;
    std::variant<std::string, int, float> data;
};