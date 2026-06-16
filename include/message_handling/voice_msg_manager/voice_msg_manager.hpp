#pragma once

#include <string>
#include <tgbot/tgbot.h>

#include "queues/message_queue.hpp"
#include "common/message.hpp"

class VoiceMsgManager {
public:
    VoiceMsgManager(MessageQueue<RawMessage>& main_queue, TgBot::Bot& bot)
    :m_main_queue(main_queue), m_recording_queue("recordings_path"), m_bot(bot){};

    void process_recordings();

    void push(const RawMessage& raw_msg);
    void shutdown();

private:
    MessageQueue<RawMessage>& m_main_queue;
    MessageQueue<RawMessage> m_recording_queue;
    TgBot::Bot& m_bot;
};
