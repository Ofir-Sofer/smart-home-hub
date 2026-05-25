#pragma once

#include <string>

#include "queues/message_queue.hpp"

class Listener {
public:
    Listener(MessageQueue<std::string>& main_queue)
    :m_main_queue(main_queue){};

    void push_to_main_queue(const std::string& user_input);

private:
    MessageQueue<std::string>& m_main_queue;
};
