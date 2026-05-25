#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include "devices/idevice.hpp"

class DeviceFactory {
public:
    DeviceFactory(const std::string& config_path);
    
    // Returns a non-owning pointer. Do not delete.
    // Lifetime is tied to the Devicefactory instance.
    [[nodiscard]] IDevice* get_device(const std::string& device_id);
    std::vector<std::string> get_device_id_list();

private:
    using DeviceConstructor = std::function<std::unique_ptr<IDevice>(const std::string&)>;
    using ConstructorMap = std::unordered_map<std::string, DeviceConstructor>;
    std::unordered_map<std::string, std::unique_ptr<IDevice>> m_device_map;
    ConstructorMap register_constructors();
    ConstructorMap m_device_constructors;
};