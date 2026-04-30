#include <iostream>

#include "tests/test_queue.cpp"
#include "tests/test_message.cpp"

int main() {
    std::cout << "Smart Home Hub starting...\n";
    run_queue_tests();
    run_message_tests();
    return 0;
}
