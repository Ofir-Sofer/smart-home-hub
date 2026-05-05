#include <string>
#include <unordered_map>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

#include "device_management/factory/device_factory.hpp"
#include "devices/idevice.hpp"
#include "devices/dummy_device.hpp"

DeviceFactory::DeviceFactory(const std::string& config_path) {
    m_device_constructors = register_constructors();
    std::ifstream file(config_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open devices.json");
    }
    nlohmann::json j;
    file >> j;
    for (const auto& device : j["devices"]) {
        std::string device_id = device["device_id"];
        std::string device_type = device["device_type"];
        auto it = m_device_constructors.find(device_type);
        if (it != m_device_constructors.end()) {
            m_device_map[device_id] = it->second(device_id);
        } else {
            throw std::runtime_error("Unknown device type: " + device_type);
        }
    }
}

IDevice* DeviceFactory::get_device(const std::string &device_id) {
    auto it = m_device_map.find(device_id);
    if (it != m_device_map.end()) {
        return it->second.get();
    }
    return nullptr;
}
std::unordered_map<std::string,
                   std::function<std::unique_ptr<IDevice>(const std::string &)>>
DeviceFactory::register_constructors() {
    std::unordered_map<std::string,
                   std::function<std::unique_ptr<IDevice>(const std::string &)>> device_constructors;
    device_constructors["dummy_device"] = [](const std::string& device_id) {
        return std::make_unique<DummyDevice>(device_id);
    };
    return device_constructors;
}