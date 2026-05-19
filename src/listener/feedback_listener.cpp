#include <string>

#include "listener/feedback_listener.hpp"
#include "common/device_result.hpp"

void FeedbackListener::forward_to_user(const DeviceResult& device_res, const std::string& user_id, const std::string& device_id) {
    m_server.route_to_user(device_res, user_id, device_id);
}