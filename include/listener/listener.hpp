#pragma once

#include <string>

#include "queues/message_queue.hpp"

// FeedbackListener is responsible for handling all messages that flow from devices back to the user.
// Currently it forwards DeviceResult directly to the Server, but is designed to grow into a richer layer.
// Future responsibilities may include:
// - Formatting device responses for Telegram
// - Handling AWAITING_CONFIRMATION responses (pausing queue, presenting options to user)
// - Logging device feedback for debugging and analytics
// - Filtering or prioritizing feedback messages
class Listener {
public:
    Listener(MessageQueue<std::string>& main_queue)
    :m_main_queue(main_queue){};

    void push_to_main_queue(const std::string& user_input);

private:
    MessageQueue<std::string>& m_main_queue;
};
