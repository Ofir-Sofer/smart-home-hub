#include <string>
#include <iostream>
#include <optional>

#include "test_listener.hpp"
#include "listener/listener.hpp"
#include "queues/message_queue.hpp"

bool test_listener_push(){
    std::string device_id = "dummy_test";
    MessageQueue<std::string> main_queue(device_id);
    Listener listener(main_queue);
    std::string cmd = "success";
    std::string user_input = device_id + ":" + cmd;
    listener.push_to_main_queue(user_input);
    std::optional<std::string> wrapped_poped_input = main_queue.pop();
    return wrapped_poped_input.value_or("") == user_input;
}

void run_listener_tests() {
    std::cout << "START LISTENER TESTS:\n";
    std::cout << "test_listener_push: " << (test_listener_push() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END LISTENER TESTS\n";
}