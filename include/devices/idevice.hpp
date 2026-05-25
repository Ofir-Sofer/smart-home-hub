#pragma once

#include <string>

#include "common/message.hpp"
#include "common/device_result.hpp"

class IDevice {
public:
    virtual ~IDevice() = default;

    virtual DeviceResult process_command(const Message& input_msg) = 0;
protected:
    IDevice(const std::string& device_id) : m_device_id(device_id) {};
    std::string m_device_id;
};