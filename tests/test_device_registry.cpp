#include <iostream>
#include <string>

#include "device_management/registry/device_registry.hpp"
#include "device_management/factory/device_factory.hpp"
#include "devices/idevice.hpp"

bool test_registry_valid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string device_id = "dummy_test";
    DeviceRegistry device_registry(device_factory);
    IDevice* valid_device = device_registry.get_device(device_id);
    return valid_device != nullptr;
}

bool test_registry_invalid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string device_id = "non_existent";
    DeviceRegistry device_registry(device_factory);
    IDevice* invalid_device = device_registry.get_device(device_id);
    return invalid_device == nullptr;
}

bool test_device_queue_valid() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    DeviceRegistry device_registry(device_factory);
    std::string dummy_id = "dummy_test";
    return device_registry.get_queue(dummy_id) != nullptr;
}

bool test_device_queue_invalid() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    DeviceRegistry device_registry(device_factory);
    std::string dummy_id = "invalid_device";
    return device_registry.get_queue(dummy_id) == nullptr;
}


void run_device_registry_tests() {
    std::cout << "START DEVICE REGISTRY TESTS:\n";
    std::cout << "test_registry_valid_device: " << (test_registry_valid_device() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_registry_invalid_device: " << (test_registry_invalid_device() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_device_queue_valid: " << (test_device_queue_valid() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_device_queue_invalid: " << (test_device_queue_invalid() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END DEVICE REGISTRY TESTS\n";
}