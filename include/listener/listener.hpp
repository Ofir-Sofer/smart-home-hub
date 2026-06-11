#pragma once

#include <string>
#include <unordered_set>
#include <atomic>
#include <cstdint> // for int64_t
#include <tgbot/tgbot.h>

#include "queues/message_queue.hpp"
#include "common/message.hpp"
#include "message_handling/voice_msg_manager/voice_msg_manager.hpp"

class Listener {
public:
    Listener(MessageQueue<RawMessage>& main_queue, VoiceMsgManager& voice_manager, TgBot::Bot& bot, const std::string& authorized_users_path);

    void start();
    void shutdown();
    
private:
    void push_to_main_queue(const RawMessage& user_input);
    MessageQueue<RawMessage>& m_main_queue;
    VoiceMsgManager& m_voice_manager;
    TgBot::Bot& m_bot;
    std::unordered_set<int64_t> m_authorized_users;
    std::atomic<bool> m_shutdown{false};
    bool is_authorized(int64_t user_id) const;
};
