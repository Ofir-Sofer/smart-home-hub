#include <iostream>
#include <algorithm>

#include "device_management/factory/device_factory.hpp"
#include "devices/idevice.hpp"

bool test_valid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    IDevice* valid_device = device_factory.get_device("dummy_test");
    return valid_device != nullptr;
}

bool test_invalid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    IDevice* invalid_device = device_factory.get_device("non_existent");
    return invalid_device == nullptr;
}

bool test_invalid_config_path() {
    std::string conf_path = "config/non_existent.json";
    try {
        DeviceFactory device_factory(conf_path);
        return false;
    } catch(const std::runtime_error& e) {
        return true;
    }
}

bool test_device_list() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string dummy_id = "dummy_test";
    std::vector<std::string> device_id_list = device_factory.get_device_id_list();
    if (std::find(device_id_list.begin(), device_id_list.end(), dummy_id) != device_id_list.end()) {
        return true;
    }
    return false;
}

void run_device_factory_tests() {
    std::cout << "START DEVICE FACTORY TESTS:\n";
    std::cout << "test_valid_device: " << (test_valid_device() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_invalid_device: " << (test_invalid_device() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_invalid_config_path: " << (test_invalid_config_path() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_device_list: " << (test_device_list() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END DEVICE FACTORY TESTS\n";
}