#include <utility>

#include "device_management/registry/device_registry.hpp"
#include "device_management/factory/device_factory.hpp"
#include "queues/message_queue.hpp"

DeviceRegistry::DeviceRegistry(DeviceFactory& factory)
    : m_factory(factory) {
    m_device_id_list = m_factory.get_device_id_list();
    for (const auto& id : m_device_id_list) {
        m_device_queues.emplace(id,id);
    }
}

IDevice* DeviceRegistry::get_device(const std::string& device_id) const {
    return m_factory.get_device(device_id);
}

MessageQueue<Message>* DeviceRegistry::get_queue(const std::string &device_id) {
    auto it = m_device_queues.find(device_id);
    if (it != m_device_queues.end()) {
        return &(it->second);
    }
    return nullptr;
}

std::vector<std::string> DeviceRegistry::get_device_id_list() const {
    return m_device_id_list;
}

std::string DeviceRegistry::get_description(const std::string &device_id) const {
    return m_factory.get_description(device_id);
}

std::vector<std::string> DeviceRegistry::get_commands(const std::string &device_id) const {
    return m_factory.get_commands(device_id);
}

std::vector<std::string> DeviceRegistry::get_command_values(const std::string &device_id, const std::string &command) const {
    return m_factory.get_command_values(device_id, command);
}

void DeviceRegistry::shutdown_all_queues() {
    for (auto& it : m_device_queues) {
        it.second.shutdown();
    }
}
