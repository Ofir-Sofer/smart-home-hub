#include <utility>

#include "device_management/registry/device_registry.hpp"
#include "device_management/factory/device_factory.hpp"
#include "queues/message_queue.hpp"

DeviceRegistry::DeviceRegistry(DeviceFactory& factory) 
    : m_factory(factory) {
    std::vector<std::string> device_id_list = m_factory.get_device_id_list();
    for (auto it = device_id_list.begin(); it != device_id_list.end(); ++it) {
        m_device_queues.emplace(*it,*it);
    }
}

IDevice* DeviceRegistry::get_device(const std::string& device_id) {
    return m_factory.get_device(device_id);
}

MessageQueue<Message>* DeviceRegistry::get_queue(const std::string &device_id) {
    auto it = m_device_queues.find(device_id);
    if (it != m_device_queues.end()) {
        return &(it->second);
    }
    return nullptr;
}