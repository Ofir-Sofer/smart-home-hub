#pragma once

#include <string>
#include <vector>
#include <sys/types.h>

#include "devices/idevice.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

// Note: TadiranDevice is tested via integration only.
// Unit testing requires a live AC connection and Python bridge.
// Verified manually end-to-end via Telegram commands.

class Tadiran : public IDevice {
public:
    Tadiran(const std::string& device_id);
    
    ~Tadiran();
    Tadiran(const Tadiran&) = delete;
    Tadiran& operator=(const Tadiran&) = delete;
    Tadiran(Tadiran&&) = delete;
    Tadiran& operator=(Tadiran&&) = delete;
    
    DeviceResult process_command(const Message& input_msg) override;
    std::vector<std::string> get_commands() const override;

private:
    std::string m_bridge_ip;
    int m_bridge_port;
    pid_t m_bridge_pid;
};