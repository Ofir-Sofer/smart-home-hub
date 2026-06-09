#include <iostream>
#include <string>

#include "server/server.hpp"
#include "device_management/registry/device_registry.hpp"
#include "device_management/factory/device_factory.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"
#include "devices/idevice.hpp"


bool test_server_push_to_queue() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string device_id = "dummy_test";
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    std::string cmd = "turn on";
    int64_t user_id = 0;
    Direction target = Direction::TO_DEVICE;
    Message msg = {device_id, cmd, user_id, target};
    server.push_to_device_queue(msg);
    MessageQueue<Message>* msg_q = device_registry.get_queue(device_id);
    std::optional<Message> wrapped_poped_msg = msg_q->pop();
    if (!wrapped_poped_msg.has_value()) {
        return false;
    }
    Message poped_msg = wrapped_poped_msg.value();
    return poped_msg.m_device_id == device_id;
}

bool test_server_invalid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string invalid_device_id = "invalid_device";
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    std::string cmd = "turn on";
    int64_t user_id = 0;
    Direction target = Direction::TO_DEVICE;
    Message msg = {invalid_device_id, cmd, user_id, target};
    server.push_to_device_queue(msg);
    MessageQueue<Message>* msg_q = device_registry.get_queue(invalid_device_id);
    return msg_q == nullptr;
}

void run_server_tests() {
    std::cout << "START SERVER TESTS:\n";
    std::cout << "test_server_push_to_queue: " << (test_server_push_to_queue() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_server_invalid_device: " << (test_server_invalid_device() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END SERVER TESTS\n";
}