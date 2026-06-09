#pragma once

#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

enum class ServerStatus {
    SUCCESS,
    DEVICE_NOT_FOUND
};

class Server {
public:
    Server(DeviceRegistry& device_registry)
    : m_device_registry(device_registry){}
    
    ServerStatus push_to_device_queue(const Message& msg);
    ServerStatus push_urgent_to_device_queue(const Message& msg);
    // void route_to_user(const DeviceResult& result, const std::string& user_id, const std::string& device_id);

private:
    DeviceRegistry& m_device_registry;
};
