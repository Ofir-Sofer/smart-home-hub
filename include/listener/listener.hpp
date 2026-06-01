#pragma once

#include <string>

#include <tgbot/tgbot.h>
#include "queues/message_queue.hpp"

class Listener {
public:
    Listener(MessageQueue<std::string>& main_queue, const std::string& token)
    :m_main_queue(main_queue), m_bot(token){};

    void start();
    void shutdown();
    
private:
    void push_to_main_queue(const std::string& user_input);
    MessageQueue<std::string>& m_main_queue;
    TgBot::Bot m_bot;
    std::atomic<bool> m_shutdown{false};
};
