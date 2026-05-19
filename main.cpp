#include <iostream>

#include "tests/test_queue.hpp"
#include "tests/test_message.hpp"
#include "tests/test_simple_encoder.hpp"
#include "tests/test_dummy_device.hpp"
#include "tests/test_device_factory.hpp"
#include "tests/test_device_registry.hpp"
#include "tests/test_server.hpp"
#include "tests/test_feedback_listener.hpp"
#include "tests/test_parser.hpp"

int main() {
    std::cout << "Smart Home Hub starting...\n";
    run_queue_tests();
    run_message_tests();
    run_simple_encoder_tests();
    run_dummy_device_tests();
    run_device_factory_tests();
    run_device_registry_tests();
    run_server_tests();
    run_feedback_listener_tests();
    run_parser_tests();
    return 0;
}
