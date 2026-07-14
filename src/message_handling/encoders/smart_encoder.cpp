#include <vector>
#include <string>

#include "message_handling/encoders/smart_encoder.hpp"

SmartEncoder::SmartEncoder(DeviceRegistry &registry)
    :m_registry(registry), m_device_list(create_device_list(registry)){}

// Builds a newline-separated list of available devices for stage-1 (device selection)
// prompts, one line per device in the form "- device_id: description". Pure data only —
// no instruction wording or formatting beyond the list itself; each concrete encoder
// wraps this with its own model-specific prompt phrasing.
std::string SmartEncoder::create_device_list(const DeviceRegistry& registry) {
    std::vector<std::string> device_id_list = registry.get_device_id_list();
    std::string list;
    for (auto& device_id:device_id_list) {
        list += "- " + device_id + ": " + registry.get_description(device_id) + "\n";
    }
    list.erase(list.size() - 1); //remove trailing newline
    return list;
}

// vector<string> -> comma-separated string
std::string SmartEncoder::join_comma_separated(const std::vector<std::string>& items, const std::string& separator) {
    std::string list;
    if (!items.empty()) {
        for (auto& value:items) {
            list += value + separator;
        }
        list.erase(list.size() - separator.size()); //remove trailing separator
    }
    return list;
}
