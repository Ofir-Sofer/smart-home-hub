#pragma once

#include "devices/idevice.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

class DummyDevice : public IDevice {
public:
    DummyDevice(const std::string& device_id)
    : IDevice(device_id, "config/test_devices.json") {};

protected:
    DeviceResult process_command(const Message& input_msg) override;
};
