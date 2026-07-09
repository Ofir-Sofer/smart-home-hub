#pragma once

#include <string>
#include <vector>

#include "devices/idevice.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"

// Note: Roborock vacuum is tested via integration only.
// Verified manually end-to-end via Telegram commands.

struct curl_slist;
using CURL = void;

class Roborock : public IDevice {
public:
    Roborock(const std::string& device_id);

    ~Roborock() = default;

    std::vector<std::string> get_commands() const override;

private:
    std::string m_base_url;
    std::string m_vacuum_entity_id;
    std::string m_button_entity_prefix;
    std::vector<std::string> m_routine_names;
    std::vector<std::string> m_speed_values;
    const std::string m_token;

protected:
    DeviceResult process_command(const Message& input_msg) override;
    CURL* create_curl_handle(const std::string& command, std::string& buffer, curl_slist*& headers);
};
