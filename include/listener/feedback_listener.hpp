#pragma once

#include <string>
#include <tgbot/tgbot.h>

#include "common/device_result.hpp"
#include "common/message.hpp"

// FeedbackListener now requires a live TgBot::Bot instance with a valid token.
// Telegram API calls cannot be unit tested without a real connection.
// Verified manually end-to-end via Telegram commands.
class FeedbackListener {
public:
    FeedbackListener(TgBot::Bot& bot)
    :m_bot(bot){};
    
    void forward_to_user(const DeviceResult& device_res, int64_t user_id, const std::string& device_id, const Message& msg);

private:
    TgBot::Bot& m_bot;
};