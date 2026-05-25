#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "device_management/factory/device_factory.hpp"
#include "devices/idevice.hpp"
#include "queues/message_queue.hpp"
#include "common/message.hpp"

class DeviceRegistry {
public:
    DeviceRegistry(DeviceFactory& factory);

    // Returns a non-owning pointer. Do not delete.
    // Lifetime is tied to the DeviceRegistry instance.
    [[nodiscard]] IDevice* get_device(const std::string& device_id);
    // Returns a non-owning pointer. Do not delete.
    // Lifetime is tied to the DeviceRegistry instance.
    [[nodiscard]] MessageQueue<Message>* get_queue(const std::string& device_id);

private:
    DeviceFactory& m_factory;
    std::unordered_map<std::string, MessageQueue<Message>> m_device_queues;
};