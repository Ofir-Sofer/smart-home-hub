#include <iostream>

#include "server/server.hpp"
#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "common/device_result.hpp"
#include "queues/message_queue.hpp"

ServerStatus Server::push_to_device_queue(const Message& msg) {
    MessageQueue<Message>* msg_queue = m_device_registry.get_queue(msg.m_device_id);
    if (msg_queue != nullptr) {
        msg_queue->push(msg);
        return ServerStatus::SUCCESS;
    }
    return ServerStatus::DEVICE_NOT_FOUND;
}

ServerStatus Server::push_urgent_to_device_queue(const Message& msg) {
    MessageQueue<Message>* msg_queue = m_device_registry.get_queue(msg.m_device_id);
    if (msg_queue != nullptr) {
        msg_queue->push_urgent(msg);
        return ServerStatus::SUCCESS;
    }
    return ServerStatus::DEVICE_NOT_FOUND;
}