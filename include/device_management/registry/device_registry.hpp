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
    [[nodiscard]] IDevice* get_device(const std::string& device_id) const;
    // Returns a non-owning pointer. Do not delete.
    // Lifetime is tied to the DeviceRegistry instance.
    [[nodiscard]] MessageQueue<Message>* get_queue(const std::string& device_id);
    [[nodiscard]] std::vector<std::string> get_device_id_list() const;
    [[nodiscard]] std::string get_description(const std::string& device_id) const;
    [[nodiscard]] std::vector<std::string> get_commands(const std::string& device_id) const;
    [[nodiscard]] std::vector<std::string> get_command_values(const std::string& device_id, const std::string& command) const;
    void shutdown_all_queues();

private:
    DeviceFactory& m_factory;
    std::vector<std::string> m_device_id_list;
    std::unordered_map<std::string, MessageQueue<Message>> m_device_queues;
};
