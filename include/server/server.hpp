#pragma once

#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

class Server {
public:
    Server(DeviceRegistry& device_registry)
    : m_device_registry(device_registry){}
    
    void push_to_device_queue(const Message& msg);
    void push_urgent_to_device_queue(const Message& msg);
    void route_to_user(const DeviceResult& result, const std::string& user_id, const std::string& device_id);

private:
    DeviceRegistry& m_device_registry;
};
