#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cerrno>

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
    m_bridge_ip = j.at("bridge_ip");
    m_bridge_port = j.at("port");

    //run python bridge
    m_bridge_pid = fork();
    if (m_bridge_pid == 0) {
        // we are the child — run Python
        execl("/usr/bin/python3", "python3", "scripts/tadiran_bridge.py", nullptr);
    } else if (m_bridge_pid > 0) {
        // we are the parent — wait for bridge to start
        sleep(1);
    } else {
        // fork failed
        throw std::runtime_error("Failed to fork bridge process");
    }
}

Tadiran::~Tadiran() {
    kill(m_bridge_pid, SIGTERM);
    waitpid(m_bridge_pid, nullptr, 0);
}

DeviceResult Tadiran::process_command(const Message& input_msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return {DeviceStatus::FAILURE, "Socket creation failed"};
    }

    //set timeouts
    struct timeval timeout{};
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    // Set destination
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_bridge_port);
    inet_pton(AF_INET, m_bridge_ip.c_str(), &addr.sin_addr);

    // Connect
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return {DeviceStatus::FAILURE, "Connection failed"};
    }

    // Send data
    int send_return_value = send(sock, input_msg.m_cmd.c_str(), input_msg.m_cmd.size(), 0);
    if (send_return_value < 0) {
        int send_errno = errno;
        close(sock);
        switch (send_errno) {
            case EWOULDBLOCK:
                return {DeviceStatus::FAILURE, "timed out while sending to bridge"};
            case EPIPE:
                return {DeviceStatus::FAILURE, "connection to bridge is closed"};
            case EINTR:
                return {DeviceStatus::FAILURE, "send interrupted by signal (expected during shutdown; investigate if seen otherwise)"};
            case ENOTCONN:
                return {DeviceStatus::FAILURE, "socket not connected"};
            default:
                return {DeviceStatus::FAILURE, std::string("send error: ") + std::strerror(send_errno)};
        }
    }


    // Receive response
    char buffer[1024] = {};
    int recv_return_value = recv(sock, buffer, sizeof(buffer), 0);
    int recv_errno = errno;
    close(sock);
    if (recv_return_value == 0) {
        return {DeviceStatus::FAILURE, "connection closed before response received"};
    } else if (recv_return_value < 0) {
        // switch case by values and errors
        switch (recv_errno) {
            case EWOULDBLOCK:
                return {DeviceStatus::FAILURE, "timed out waiting for response from bridge"};
            case ECONNRESET:
                return {DeviceStatus::FAILURE, "connection reset by bridge"};
            case EINTR:
                return {DeviceStatus::FAILURE, "receive interrupted by signal (expected during shutdown; investigate if seen otherwise)"};
            case ENOTCONN:
                return {DeviceStatus::FAILURE, "socket not connected"};
            default:
                return {DeviceStatus::FAILURE, std::string("receive error: ") + std::strerror(recv_errno)};
        }
    }
    std::string response(buffer);
    if (response.find("SUCCESSFUL EXECUTION") != std::string::npos) {
        return {DeviceStatus::SUCCESS, "command executed"};
    } else {
        return {DeviceStatus::FAILURE, response};
    }
}
