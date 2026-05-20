#include <iostream>
#include <string>

#include "test_message.hpp"
#include "common/message.hpp"

bool test_message() {
    std::string device = "device_A";
    std::string cmd = "turn on";
    std::string user_id = "user_123";
    Direction target = Direction::TO_DEVICE;
    Message msg = {device, cmd, user_id, target};
    return msg.m_device_id == device && msg.m_cmd == cmd && msg.m_user_id == user_id && msg.m_target == target;
}

void run_message_tests() {
    std::cout << "START MESSAGE TESTS:\n";
    std::cout << "test_message: " << (test_message() ? "PASS" : "FAIL") << "\n";
    std::cout << "END MESSAGE TESTS\n";
}