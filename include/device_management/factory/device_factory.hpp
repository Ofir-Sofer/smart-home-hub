#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include "devices/idevice.hpp"

struct DeviceEntry {
    std::unique_ptr<IDevice> device;
    std::string description;
    std::unordered_map<std::string, std::vector<std::string>> commands;
};

class DeviceFactory {
public:
    DeviceFactory(const std::string& config_path);

    // Returns a non-owning pointer. Do not delete.
    // Lifetime is tied to the DeviceFactory instance.
    [[nodiscard]] IDevice* get_device(const std::string& device_id) const;
    [[nodiscard]] std::vector<std::string> get_device_id_list() const;
    [[nodiscard]] std::string get_description(const std::string& device_id) const;
    [[nodiscard]] std::vector<std::string> get_commands(const std::string& device_id) const;
    [[nodiscard]] std::vector<std::string> get_command_values(const std::string& device_id, const std::string& command) const;

private:
    using DeviceConstructor = std::function<std::unique_ptr<IDevice>(const std::string&)>;
    using ConstructorMap = std::unordered_map<std::string, DeviceConstructor>;
    std::unordered_map<std::string, DeviceEntry> m_device_map;
    ConstructorMap register_constructors();
    ConstructorMap m_device_constructors;
};
