#include <string>
#include <iostream>

#include "test_simple_encoder.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "common/message.hpp"

bool test_simple_encoder() {
    SimpleEncoder simp_enc;
    std::string device = "washer";
    std::string cmd = "turn on";
    std::string user_msg = device + ":" + cmd;
    std::string user_id = "def_user";
    Direction target = Direction::TO_DEVICE;
    Message msg = simp_enc.encode(user_msg);
    return msg.m_device_id == device && msg.m_cmd == cmd && msg.m_user_id == user_id && msg.m_target == target;
}

void run_simple_encoder_tests() {
    std::cout << "START SIMPLE ENCODER TESTS:\n";
    std::cout << "test_simple_encoder: " << (test_simple_encoder() ? "PASS" : "FAIL") << "\n";
    std::cout << "END SIMPLE ENCODER TESTS\n";
}