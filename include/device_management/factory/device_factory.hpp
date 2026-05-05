#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

#include "devices/idevice.hpp"

class DeviceFactory {
public:
    DeviceFactory(const std::string& config_path);
    IDevice* get_device(const std::string& device_id);
private:
    std::unordered_map<std::string, std::function<std::unique_ptr<IDevice>(const std::string&)>> register_constructors();
    std::unordered_map<std::string, std::unique_ptr<IDevice>> m_device_map;
    std::unordered_map<std::string, std::function<std::unique_ptr<IDevice>(const std::string&)>> m_device_constructors;
};