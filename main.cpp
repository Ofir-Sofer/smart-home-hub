#include <iostream>

#include "tests/test_queue.hpp"
#include "tests/test_message.hpp"

int main() {
    std::cout << "Smart Home Hub starting...\n";
    run_queue_tests();
    run_message_tests();
    return 0;
}
