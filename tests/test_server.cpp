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
    std::string user_id = "user_123";
    Direction target = Direction::TO_DEVICE;
    Message msg = {device_id, cmd, user_id, target};
    server.push_to_device_queue(msg);
    MessageQueue<Message>* msg_q = device_registry.get_queue(device_id);
    Message poped_msg = msg_q->pop();
    return poped_msg.m_device_id == device_id;
}

bool test_server_invalid_device() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string invalid_device_id = "invalid_device";
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    std::string cmd = "turn on";
    std::string user_id = "user_123";
    Direction target = Direction::TO_DEVICE;
    Message msg = {invalid_device_id, cmd, user_id, target};
    server.push_to_device_queue(msg);
    MessageQueue<Message>* msg_q = device_registry.get_queue(invalid_device_id);
    return msg_q == nullptr;
}

bool test_server_route_to_user(){
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string device_id = "dummy_test";
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    DeviceResult device_res = {DeviceStatus::SUCCESS, 1};
    std::string user_id = "user123";
    server.route_to_user(device_res, user_id, device_id);
    // route_to_user currently prints to console - no return value to verify
    // test just ensures it doesn't crash
    return true;
}

void run_server_tests() {
    std::cout << "START SERVER TESTS:\n";
    std::cout << "test_server_push_to_queue: " << (test_server_push_to_queue() ? "PASS" : "FAIL") << "\n";
    std::cout << "test_server_invalid_device: " << (test_server_invalid_device() ? "PASS" : "FAIL") << "\n";
    std::cout << "test_server_route_to_user: " << (test_server_route_to_user() ? "PASS" : "FAIL") << "\n";
    std::cout << "END SERVER TESTS\n";
}