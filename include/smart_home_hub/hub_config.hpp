#pragma once

#include <string>

struct EncoderSettings {
    std::string type;
    std::string model_path;
};

class HubConfig {
public:
    HubConfig();

    const std::string& get_tg_token() const;
    const std::string& get_temp_path() const;
    const std::string& get_authorized_users_path() const;
    const std::string& get_device_config_path() const;
    const EncoderSettings& get_encoder_settings() const;

private:
    std::string m_settings_path = "config/settings.json";
    std::string m_tg_token;
    std::string m_temp_path = "/tmp/smart_home_hub/audio";
    std::string m_authorized_users_path = "config/authorized_users.json";
    std::string m_device_config_path = "config/devices.json";
    EncoderSettings m_encoder_settings;
};
