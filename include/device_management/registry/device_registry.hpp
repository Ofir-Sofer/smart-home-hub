#pragma once

#include <string>

#include "device_management/factory/device_factory.hpp"
#include "devices/idevice.hpp"

class DeviceRegistry {
public:
    DeviceRegistry(DeviceFactory& factory)
        :m_factory(factory) {}

    IDevice* get_device(const std::string& device_id);


private:
    DeviceFactory& m_factory;
};