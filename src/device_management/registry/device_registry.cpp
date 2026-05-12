#include "device_management/registry/device_registry.hpp"

IDevice* DeviceRegistry::get_device(const std::string& device_id) {
    return m_factory.get_device(device_id);
}