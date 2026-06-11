#include <string>
#include <exception>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <tgbot/tgbot.h>
#include <cstdint>

#include "listener/listener.hpp"

Listener::Listener(MessageQueue<RawMessage>& main_queue, VoiceMsgManager& voice_manager, TgBot::Bot& bot, const std::string& authorized_users_path)
    :m_main_queue(main_queue), m_voice_manager(voice_manager), m_bot(bot){
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

void Listener::push_to_main_queue(const RawMessage& user_input) {
    m_main_queue.push(user_input);
}

void Listener::start() {
    // register callback ONCE before loop
    m_bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
        int64_t user_id = message->chat->id;
        if(is_authorized(user_id)) {
            if (message->voice) {
                // handle voice message
                // push to recordings queue
                std::string file_id = message->voice->fileId;
                TgBot::File::Ptr file_info = m_bot.getApi().getFile(file_id);
                std::string file_content = m_bot.getApi().downloadFile(file_info->filePath);
                std::string file_path = "/tmp/smart_home_hub/audio/" + file_id + ".ogg";
                std::ofstream file(file_path, std::ios::binary);
                file.write(file_content.c_str(), file_content.size());
                file.close();
                RawMessage raw_msg = {file_path, user_id};
                m_voice_manager.push(raw_msg);
            } else {
                RawMessage raw_msg = {message->text, user_id};
                push_to_main_queue(raw_msg);
            }
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

bool Listener::is_authorized(int64_t user_id) const{
    return m_authorized_users.find(user_id) != m_authorized_users.end();
}