#pragma once

#include <string>

#include "server/server.hpp"
#include "common/device_result.hpp"

// FeedbackListener is responsible for handling all messages that flow from devices back to the user.
// Currently it forwards DeviceResult directly to the Server, but is designed to grow into a richer layer.
// Future responsibilities may include:
// - Formatting device responses for Telegram
// - Handling AWAITING_CONFIRMATION responses (pausing queue, presenting options to user)
// - Logging device feedback for debugging and analytics
// - Filtering or prioritizing feedback messages
class FeedbackListener {
public:
    FeedbackListener(Server& server)
    :m_server(server){};
    
    void forward_to_user(const DeviceResult& device_res, const std::string& user_id, const std::string& device_id);

private:
    Server& m_server;
};