#pragma once

#include <string>
#include <cstdint>

#include "message_handling/encoders/iencoder.hpp"
#include "device_management/registry/device_registry.hpp"

class SmartEncoder : public IEncoder {
public:
    virtual ~SmartEncoder() = default;

protected:
    SmartEncoder(DeviceRegistry& registry);
    DeviceRegistry& m_registry;
    const std::string m_per_device_commands_list;

private:
    static std::string create_device_list(const DeviceRegistry& registry);
};