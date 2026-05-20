#include <string>
#include <iostream>

#include "test_feedback_listener.hpp"
#include "listener/feedback_listener.hpp"
#include "device_management/factory/device_factory.hpp"
#include "device_management/registry/device_registry.hpp"
#include "server/server.hpp"
#include "common/device_result.hpp"
#include "test_feedback_listener.hpp"


bool test_feedback_listener_route_to_user() {
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    std::string device_id = "dummy_test";
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    FeedbackListener feedback_lis(server);
    DeviceResult device_res = {DeviceStatus::SUCCESS, 1};
    std::string user_id = "user123";
    feedback_lis.forward_to_user(device_res, user_id, device_id);
    // route_to_user currently prints to console - no return value to verify
    // test just ensures it doesn't crash
    return true;
}

void run_feedback_listener_tests() {
    std::cout << "START FEEDBACK LISTENER TESTS:\n";
    std::cout << "test_feedback_listener_route_to_user: " << (test_feedback_listener_route_to_user() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END FEEDBACK LISTENER TESTS\n";
}