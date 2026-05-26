#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <optional>

#include "test_queue.hpp"
#include "queues/message_queue.hpp"
#include "common/message.hpp"

bool test_queue_empty() {
    MessageQueue<std::string> msg_q("testing");
    return msg_q.empty();
}

bool test_queue_push_and_pop() {
    std::string msg = "this is a message";
    MessageQueue<std::string> msg_q("testing");
    msg_q.push(msg);
    std::optional<std::string> wrapped_pop_msg = msg_q.pop();
    return wrapped_pop_msg.value_or("") == msg;
}

bool test_queue_push_pop_order() {
    const int msg_count = 3;
    MessageQueue<int> msg_q("testing");
    for (int i=0 ; i<msg_count ; i++) {
        msg_q.push(i);
    }
    bool ret_val = true;
    std::optional<int> wrapped_pop_msg;
    for (int i=0 ; i<msg_count ; i++) {
        wrapped_pop_msg = msg_q.pop();
        if (wrapped_pop_msg.value_or(999) != i) {
            ret_val = false;
            break;
        }
    }
    return ret_val;
}

bool test_queue_push_urgent() {
    //this make sure that an urgent test gets the priority even if more messages got pushed after the urgent
    std::string msg1 = "regular message #1";
    std::string msg2 = "regular message #2";
    std::string urgent_msg = "urgent message";
    MessageQueue<std::string> msg_q("testing");
    msg_q.push(msg1);
    msg_q.push_urgent(urgent_msg);
    msg_q.push(msg2);
    std::optional<std::string> wrapped_pop_msg = msg_q.pop();
    return wrapped_pop_msg.value_or("") == urgent_msg;
}

bool test_queue_threads() {
    MessageQueue<std::string> msg_q("testing");
    std::optional<std::string> wrapped_pop_msg;

    std::thread t1([&msg_q, &wrapped_pop_msg](){
        wrapped_pop_msg = msg_q.pop(); //q is empty so it should wait untill a new val is inserted
    });

    std::this_thread::sleep_for(std::chrono::seconds(3)); //wait 3 seconds before pushing a new message
    std::string msg = "this is a message";
    msg_q.push(msg);
    t1.join();
    return wrapped_pop_msg.value_or("") == msg;
}

void run_queue_tests() {
    std::cout << "START QUEUE TESTS:\n";
    std::cout << "test_queue_empty: " << (test_queue_empty() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_queue_push_and_pop: " << (test_queue_push_and_pop() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_queue_push_pop_order: " << (test_queue_push_pop_order() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_queue_push_urgent: " << (test_queue_push_urgent() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "test_queue_threads: " << (test_queue_threads() ? "PASS" : "!!!!!!!!!!FAIL!!!!!!!!!!") << "\n";
    std::cout << "END QUEUE TESTS\n";
}