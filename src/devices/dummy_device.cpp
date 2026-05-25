#include "devices/dummy_device.hpp"
#include "common/message.hpp"

DeviceResult DummyDevice::process_command(const Message& input_msg) {
    DeviceResult res;
    if (input_msg.m_cmd == "success") {
        res.m_data = "success";
        res.m_status = DeviceStatus::SUCCESS;
    } else if (input_msg.m_cmd == "fail") {
        res.m_data = "fail";
        res.m_status = DeviceStatus::FAILURE;
    } else {
        res.m_data = "waiting for response";
        res.m_status = DeviceStatus::AWAITING_CONFIRMATION;
    }
    return res;
}