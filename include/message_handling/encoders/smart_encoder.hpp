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

    [[nodiscard]] static std::string join_comma_separated(const std::vector<std::string>& items, const std::string& separator = ", ");

    DeviceRegistry& m_registry;
    const std::string m_device_list;

private:
    static std::string create_device_list(const DeviceRegistry& registry);
};
