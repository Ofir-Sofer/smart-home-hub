#include "devices/dummy_device.hpp"
#include "common/message.hpp"

DeviceResult DummyDevice::process_command(const Message& input_msg) {
    DeviceResult res;
    if (input_msg.m_cmd == "success" || input_msg.m_cmd == "activate cleaner") {
        res.data = "success";
        res.status = DeviceStatus::SUCCESS;
    } else if (input_msg.m_cmd == "fail") {
        res.data = "fail";
        res.status = DeviceStatus::FAILURE;
    } else {
        res.data = "waiting for response";
        res.status = DeviceStatus::AWAITING_CONFIRMATION;
    }
    return res;
}