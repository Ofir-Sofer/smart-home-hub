#include "devices/dummy_device.hpp"
#include "common/message.hpp"

DeviceResult DummyDevice::process_command(const Message& input_msg) {
    if (input_msg.m_cmd == "success") {
        return {DeviceStatus::SUCCESS, "success"};
    } else if (input_msg.m_cmd == "fail") {
        return {DeviceStatus::FAILURE, "fail"};
    } else {
        return {DeviceStatus::AWAITING_CONFIRMATION, "waiting for response"};
    }
}

std::vector<std::string> DummyDevice::get_commands() const {
    return {"success", "fail", "waiting for response"};
}