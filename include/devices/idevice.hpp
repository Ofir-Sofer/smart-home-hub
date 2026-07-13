#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>

#include "common/message.hpp"
#include "common/device_result.hpp"

class IDevice {
public:
    virtual ~IDevice() = default;

    DeviceResult safe_execution(const Message& input_msg);

protected:
    IDevice(const std::string& device_id);

    virtual DeviceResult process_command(const Message& input_msg) = 0;
    // Returns true if input_msg.m_cmd's command exists and, if it takes a
    // value, that value is legal (exact match for discrete lists, in-bounds
    // for a "range(min,max)" entry). Derived classes call this at the top
    // of process_command before doing any actual device work.

    // Populated by each derived class's constructor from devices.json.
    std::unordered_map<std::string, std::vector<std::string>> m_commands;
    std::string m_device_id;
    mutable std::mutex m_mutex;
};
