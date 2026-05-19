#include <iostream>
#include <variant>

#include "server/server.hpp"
#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"
#include "queues/message_queue.hpp"

void Server::push_to_device_queue(const Message& msg) {
    MessageQueue<Message>* msg_queue = m_device_registry.get_queue(msg.m_device_id);
    if (msg_queue != nullptr) {
        msg_queue->push(msg);
    }
}

void Server::push_urgent_to_device_queue(const Message& msg) {
    MessageQueue<Message>* msg_queue = m_device_registry.get_queue(msg.m_device_id);
    if (msg_queue != nullptr) {
        msg_queue->push_urgent(msg);
    }
}

void Server::route_to_user(const DeviceResult &result, const std::string &user_id, const std::string& device_id) {
    std::visit([&device_id] (const auto& res_data) {
        std::cout<< "user data: " << res_data << ", device- " << device_id << "\n";
    }, result.data);
}