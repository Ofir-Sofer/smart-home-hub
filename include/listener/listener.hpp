#pragma once

#include <string>
#include <unordered_set>
#include <cstdint> // for int64_t

#include <tgbot/tgbot.h>
#include "queues/message_queue.hpp"

class Listener {
public:
    Listener(MessageQueue<std::string>& main_queue,
        const std::string& token,
        const std::string& authorized_users_path);

    void start();
    void shutdown();
    
private:
    void push_to_main_queue(const std::string& user_input);
    MessageQueue<std::string>& m_main_queue;
    TgBot::Bot m_bot;
    std::unordered_set<int64_t> m_authorized_users;
    std::atomic<bool> m_shutdown{false};
    bool is_authorized(int64_t user_id);
};
