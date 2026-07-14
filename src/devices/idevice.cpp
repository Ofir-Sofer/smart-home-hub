#include <mutex>
#include <nlohmann/json.hpp>
#include <fstream>

#include "devices/idevice.hpp"
#include "common/device_result.hpp"

IDevice::IDevice(const std::string& device_id, const std::string& devices_settings)
:m_device_id(device_id) {
    std::ifstream devices_file(devices_settings);
    if (!devices_file.is_open()) {
        throw std::runtime_error("Could not open: " + devices_settings);
    }
    nlohmann::json devices_json;
    devices_file >> devices_json;
    m_commands = devices_json.at("devices").at(device_id).at("commands");
}

DeviceResult IDevice::safe_execution(const Message &input_msg) {
    std::unique_lock<std::mutex> lock(m_mutex);
    return process_command(input_msg);
}
