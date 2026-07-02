#include <vector>
#include <string>

#include "message_handling/encoders/smart_encoder.hpp"

SmartEncoder::SmartEncoder(DeviceRegistry &registry) 
    :m_registry(registry), m_per_device_commands_list(create_device_list(registry)){}

std::string SmartEncoder::create_device_list(const DeviceRegistry& registry) {
    std::vector<std::string> device_id_list = registry.get_device_id_list();
    std::string list = "Available devices: ";
    for (auto& device_id:device_id_list) {
        std::string per_device_commands = device_id + " [";
        std::vector<std::string> device_commands = registry.get_device(device_id)->get_commands();
        for (auto& command:device_commands) {
            per_device_commands = per_device_commands + command + ", ";
        }
        per_device_commands.erase(per_device_commands.size() - 2); //remove trailing space and comma
        per_device_commands = per_device_commands + "], ";
        list = list + per_device_commands;
    }
    list.erase(list.size() - 2); //remove trailing space and comma

    return list;
}