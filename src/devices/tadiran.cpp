#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "devices/tadiran.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"


Tadiran::Tadiran(const std::string &device_id) 
:IDevice(device_id) {
    std::string settings_path = "config/tadiran_config.json";
    std::ifstream file(settings_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open: " + settings_path);
        }
        nlohmann::json j;
        file >> j;
        m_bridge_ip = j["bridge_ip"];
        m_bridge_port = j["port"];
}

DeviceResult Tadiran::process_command(const Message &input_msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return {DeviceStatus::FAILURE, "Socket creation failed"};
    }

    // Set destination
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_bridge_port);
    inet_pton(AF_INET, m_bridge_ip.c_str(), &addr.sin_addr);

    // Connect
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed\n";
        return {DeviceStatus::FAILURE, "Connection failed"};
    }

    // Send data
    send(sock, input_msg.m_cmd.c_str(), input_msg.m_cmd.size(), 0);

    // Receive response
    char buffer[1024] = {};
    recv(sock, buffer, sizeof(buffer), 0);
    std::string response(buffer);
    close(sock);
    if (response.find("OK") != std::string::npos) {
        return {DeviceStatus::SUCCESS, "command executed"};
    } else {
        return {DeviceStatus::FAILURE, "error message"};
    }
}

std::vector<std::string> Tadiran::get_commands() const {
    std::vector<std::string> commands_list = {"on", "off", "set_temp", "set_mode", "set_fan"};
    return commands_list;
}