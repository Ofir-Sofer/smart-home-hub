#pragma once

#include <string>
#include <vector>

#include "devices/idevice.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

class Tadiran : public IDevice {
public:
    Tadiran(const std::string& device_id);
    DeviceResult process_command(const Message& input_msg) override;
    std::vector<std::string> get_commands() const override;

private:
    std::string m_bridge_ip;
    int m_bridge_port;
};