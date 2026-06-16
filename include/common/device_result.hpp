#pragma once

#include <string>

enum class DeviceStatus {
    SUCCESS,
    FAILURE,
    AWAITING_CONFIRMATION
};


struct DeviceResult {
    DeviceStatus m_status;
    std::string m_data;
};