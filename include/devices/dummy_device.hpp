#pragma once

#include "devices/idevice.hpp"

class DummyDevice : public IDevice {
public:
    DummyDevice(const std::string& device_id) : IDevice(device_id) {};
    DeviceResult process_command(const Message& input_msg) override;
};