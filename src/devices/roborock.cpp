#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <curl/curl.h>
#include <algorithm>

#include "devices/roborock.hpp"

namespace { 
    size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata); 
    std::string stringify_list(const std::vector<std::string>& values);
    std::string get_ha_token();
}

Roborock::Roborock(const std::string &device_id)
:IDevice(device_id), m_token(get_ha_token()) {
    std::string settings_path = "config/roborock_config.json";
    std::ifstream file(settings_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open: " + settings_path);
    }
    nlohmann::json j;
    file >> j;
    m_vacuum_entity_id = j["vacuum_entity_id"];
    m_button_entity_prefix = j["button_entity_prefix"];
    m_routine_names = j["routine_names"];
    m_speed_values = j["speed_values"];
    m_base_url = j["base_url"];

    std::string buffer;
    const std::string full_command = m_base_url + "/api/states/" + m_vacuum_entity_id;
    struct curl_slist* headers = nullptr;
    CURL* curl = create_curl_handle(full_command, buffer, headers);
    if (curl == nullptr) {
        throw std::runtime_error("Fail to initialize curl handle");
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl); 
    if (res != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl failed: " + std::string(curl_easy_strerror(res)));
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        throw std::runtime_error("HTTP/application error");
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
}

DeviceResult Roborock::process_command(const Message& input_msg) {
    std::string buffer;
    size_t split_ind = input_msg.m_cmd.find(':');
    std::string command_type = input_msg.m_cmd.substr(0,split_ind);
    std::string full_command;
    nlohmann::json postfields;
    if (command_type == "run_routine") {
        std::string routine = input_msg.m_cmd.substr(split_ind+1);
        if (std::find(m_routine_names.begin(), m_routine_names.end(), routine) == m_routine_names.end()) {
            std::cerr << "Routine doesnt exist\n";
            return {DeviceStatus::FAILURE, "Routine doesnt exist"};
        }
        full_command = m_base_url + "/api/services/button/press";
        postfields = {{"entity_id", m_button_entity_prefix + routine}};
    } else {
        if (command_type == "set_fan_speed") {
            std::string speed = input_msg.m_cmd.substr(split_ind+1);
            if (std::find(m_speed_values.begin(), m_speed_values.end(), speed) == m_speed_values.end()) {
                std::cerr << "Wrong fan speed value\n";
                return {DeviceStatus::FAILURE, "Wrong fan speed value"};
            }
            postfields = {{"entity_id", m_vacuum_entity_id}, {"fan_speed", speed}};
            full_command = m_base_url + "/api/services/vacuum/" + command_type;
        } else {
            if (split_ind != std::string::npos) {
                std::cerr << "Wrong command format\n";
                return {DeviceStatus::FAILURE, "Wrong command format"};
            }
            postfields = {{"entity_id", m_vacuum_entity_id}};
            full_command = m_base_url + "/api/services/vacuum/" + input_msg.m_cmd;
        }
    }
    struct curl_slist* headers = nullptr;
    CURL* curl = create_curl_handle(full_command, buffer, headers);
    if (curl == nullptr) {
        return {DeviceStatus::FAILURE, "Fail to initialize curl handle"};
    }
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    
    std::string postfields_str = postfields.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields_str.c_str());
    
    CURLcode res = curl_easy_perform(curl); 
    if (res != CURLE_OK) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return {DeviceStatus::FAILURE, "curl failed: " + std::string(curl_easy_strerror(res))};
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return {DeviceStatus::FAILURE, "HTTP/application error"};
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return {DeviceStatus::SUCCESS, "command executed"};
}

CURL* Roborock::create_curl_handle(const std::string& command, std::string& buffer, curl_slist*& headers) {
    std::string auth = "Authorization: Bearer " + m_token;
    CURL* curl = curl_easy_init();          // 1. create handle
    if(!curl) {
        return nullptr;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_URL, command.c_str());
    
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    return curl;
}

std::vector<std::string> Roborock::get_commands() const {
    std::vector<std::string> commands_list = {"start", "pause", "stop", "return_to_base", "locate", "clean_spot", "set_fan_speed:[" + stringify_list(m_speed_values) + "]", "run_routine:[" + stringify_list(m_routine_names) + "]"};
    return commands_list;
}

namespace {
    std::string get_ha_token() {
        const char* token_env = std::getenv("HA_TOKEN");
        if (!token_env) {
            throw std::runtime_error("HA token does not exist");
        }
        return std::string(token_env);
    }

    size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
        std::string* buffer = static_cast<std::string*>(userdata);
        buffer->append(ptr, size * nmemb);
        return size * nmemb;
    }

    std::string stringify_list(const std::vector<std::string>& values) {
        std::string result;
        for (const auto& v : values) {
            result += v + ",";
        }
        if (!result.empty()) {
            result.pop_back(); // remove trailing comma
        }
        return result;
    }
}