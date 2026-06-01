#include <string>
#include <exception>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

#include "listener/listener.hpp"

Listener::Listener(MessageQueue<std::string>& main_queue,
    const std::string& token,
    const std::string& authorized_users_path)
    :m_main_queue(main_queue), m_bot(token){
        std::ifstream file(authorized_users_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open: " + authorized_users_path);
        }
        nlohmann::json j;
        file >> j;
        for (const auto& user_id : j["authorized_users"]) {
            m_authorized_users.insert(user_id.get<int64_t>());
        }
    }

void Listener::push_to_main_queue(const std::string &user_input) {
    m_main_queue.push(user_input);
}

void Listener::start() {
    // register callback ONCE before loop
    m_bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
        if(is_authorized(message->chat->id)) {
            push_to_main_queue(message->text);
        } else {
            m_bot.getApi().sendMessage(message->chat->id, "This is a private bot.");
        }
    });

    // polling loop
    TgBot::TgLongPoll long_poll(m_bot, 100, 3);
    try {
        while (!m_shutdown) {
            long_poll.start();
        }
    } catch (std::exception& e) {
        std::cerr << "Listener error:" << e.what() << "\n";
    }
}

void Listener::shutdown() {
    m_shutdown = true;
}

bool Listener::is_authorized(int64_t user_id) {
    if (m_authorized_users.find(user_id) != m_authorized_users.end()) {
        return true;
    }
    return false;
}