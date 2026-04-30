#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "queues/message_queue.hpp"
#include "tests/test_queue.cpp"

bool test_message() {
    std::string device = "device_A";
    std::string cmd = "turn on";
    std::string user_id = "user_123";
    Direction target = Direction::TO_DEVICE;
    Message msg = {device, cmd, user_id, target};
    return msg.m_device_id == device && msg.m_cmd == cmd && msg.m_user_id == user_id && msg.m_target == target;
}

void run_message_tests() {
    std::cout << "test_message: " << (test_message() ? "PASS" : "FAIL") << "\n";
}