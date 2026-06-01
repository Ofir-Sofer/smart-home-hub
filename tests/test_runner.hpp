#pragma once

#include <iostream>

//test includes
void run_queue_tests();
void run_message_tests();
void run_simple_encoder_tests();
void run_dummy_device_tests();
void run_device_factory_tests();
void run_device_registry_tests();
void run_server_tests();
void run_feedback_listener_tests();
void run_parser_tests();
// Note: Listener is tested via integration only.
// Unit testing requires a live Telegram bot connection.
// Verified manually - see Telegram bot integration.

inline void run_all_tests() {
    std::cout << "Smart Home Hub start testing:\n";
    run_queue_tests();
    run_message_tests();
    run_simple_encoder_tests();
    run_dummy_device_tests();
    run_device_factory_tests();
    run_device_registry_tests();
    run_server_tests();
    run_feedback_listener_tests();
    run_parser_tests();
    std::cout << "Smart Home Hub end testing\n\n";
}