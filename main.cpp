#include <iostream>

#include "tests/test_queue.hpp"
#include "tests/test_message.hpp"
#include "tests/test_simple_encoder.hpp"

int main() {
    std::cout << "Smart Home Hub starting...\n";
    run_queue_tests();
    run_message_tests();
    run_simple_encoder_tests();
    return 0;
}
