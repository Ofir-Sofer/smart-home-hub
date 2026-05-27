#include <iostream>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

bool test_parser_proccess_message() {
    MessageQueue<std::string> user_input_queue("user_input"); // user_input is a "device id" for the raw user inputs
    SimpleEncoder encoder;
    std::string conf_path = "config/test_devices.json";
    DeviceFactory device_factory(conf_path);
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    Parser parser(user_input_queue, encoder, server);
    std::string device_id = "vacuum_sim";
    std::string cmd = "success";
    std::string user_input = device_id + ":" + cmd;
    user_input_queue.push(user_input);
    parser.process_message();
    MessageQueue<Message>* vacuum_sim_queue = device_registry.get_queue("vacuum_sim");
    std::optional<Message> wrapped_poped_msg = vacuum_sim_queue->pop();
    if (!wrapped_poped_msg.has_value()) {
        return false;
    }
    Message poped_msg = wrapped_poped_msg.value();
    std::string simple_encoder_default_user_id = "def_user";
    Direction simple_encoder_default_target = Direction::TO_DEVICE;
    return poped_msg.m_device_id == device_id && poped_msg.m_cmd == cmd && poped_msg.m_user_id == simple_encoder_default_user_id && poped_msg.m_target == simple_encoder_default_target;
}

void run_parser_tests() {
    std::cout << "START PARSER TESTS:\n";
    std::cout << "test_parser_proccess_message: " << (test_parser_proccess_message() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END PARSER TESTS\n";
}