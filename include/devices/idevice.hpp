#pragma once

#include <vector>
#include <string>
#include <mutex>

#include "common/message.hpp"
#include "common/device_result.hpp"

class IDevice {
public:
    virtual ~IDevice() = default;

    DeviceResult safe_execution(const Message& input_msg);
    virtual std::vector<std::string> get_commands() const = 0;
    
protected:
    IDevice(const std::string& device_id) : m_device_id(device_id) {};
    virtual DeviceResult process_command(const Message& input_msg) = 0;
    std::string m_device_id;
    mutable std::mutex m_mutex;
};