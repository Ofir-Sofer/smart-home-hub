#include <iostream>
#include <string>

#include "test_dummy_device.hpp"
#include "devices/dummy_device.hpp"
#include "common/device_result.hpp"
#include "common/message.hpp"


bool compare_device_results(const DeviceResult& res1, const DeviceResult& res2) {
    return res1.status == res2.status && res1.data == res2.data;
}

bool test_dummy_device() {
    std::string device_id = "def_device";
    std::string user_id = "def_user";
    Direction dir = Direction::TO_DEVICE;
    DummyDevice device(device_id);
    std::string success_data = "success";
    std::string fail_data = "fail";
    std::string user_respond_data = "waiting for response";
    Message success_msg = {device_id, success_data, user_id, dir};
    Message fail_msg = {device_id, fail_data, user_id, dir};
    Message user_respond_msg = {device_id, user_respond_data, user_id, dir};
    DeviceResult res1 = {DeviceStatus::SUCCESS, success_data};
    DeviceResult res2 = {DeviceStatus::FAILURE, fail_data};
    DeviceResult res3 = {DeviceStatus::AWAITING_CONFIRMATION, user_respond_data};
    return compare_device_results(device.process_command(success_msg), res1) && 
        compare_device_results(device.process_command(fail_msg), res2) && 
        compare_device_results(device.process_command(user_respond_msg), res3);
}

void run_dummy_device_tests() {
    std::cout << "START DUMMY DEVICE TESTS:\n";
    std::cout << "test_dummy_device: " << (test_dummy_device() ? "PASS" : "FAIL") << "\n";
    std::cout << "END DUMMY DEVICE TESTS\n";
}