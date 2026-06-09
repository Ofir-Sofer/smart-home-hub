#include <string>
#include <tgbot/tgbot.h>

#include "listener/feedback_listener.hpp"
#include "common/device_result.hpp"
#include "common/message.hpp"

void FeedbackListener::forward_to_user(const DeviceResult& device_res, int64_t user_id, const std::string& device_id, const Message& msg) {
    std::string output_msg;
    if (device_res.m_status == DeviceStatus::SUCCESS) {
        output_msg = device_id + " succeeded to run: " + msg.m_cmd;
    } else if (device_res.m_status == DeviceStatus::FAILURE) {
        output_msg = device_id + " FAILED to run: " + msg.m_cmd;
    } else {
        output_msg = device_id + " : Waiting for user intervention"; //for future development
    }
    m_bot.getApi().sendMessage(user_id, output_msg);
}