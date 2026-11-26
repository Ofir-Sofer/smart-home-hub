#include <fstream>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <stdexcept>
#include <string>

#include "smart_home_hub/hub_config.hpp"

HubConfig::HubConfig() {
    const char* tg_token = std::getenv("TELEGRAM_TOKEN");
    if (!tg_token) {
        throw std::runtime_error("TELEGRAM_TOKEN environment variable not set.");
    }
    m_tg_token = tg_token;

    try {
        std::ifstream file(m_settings_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open: " + m_settings_path);
        }
        nlohmann::json j;
        file >> j;
        m_encoder_settings.type = j.at("encoder");
        if (m_encoder_settings.type == "llama") {
            std::string active_model = j.at("active_model");
            m_encoder_settings.model_path = j.at("available_models").at(active_model).at("path");
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("settings.json error: ") + e.what());
    }
}

const std::string& HubConfig::get_tg_token() const {
    return m_tg_token;
}

const std::string& HubConfig::get_temp_path() const {
    return m_temp_path;
}

const std::string& HubConfig::get_authorized_users_path() const {
    return m_authorized_users_path;
}

const std::string& HubConfig::get_device_config_path() const {
    return m_device_config_path;
}

const EncoderSettings& HubConfig::get_encoder_settings() const {
    return m_encoder_settings;
}
