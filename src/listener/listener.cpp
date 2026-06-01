#include <string>
#include <exception>

#include "listener/listener.hpp"

void Listener::push_to_main_queue(const std::string &user_input) {
    m_main_queue.push(user_input);
}

void Listener::start() {
    // register callback ONCE before loop
    m_bot.getEvents().onAnyMessage([&](TgBot::Message::Ptr message) {
        push_to_main_queue(message->text);
    });

    // polling loop
    TgBot::TgLongPoll long_poll(m_bot, 100, 3);
    try {
        while (!m_shutdown) {
            long_poll.start();
        }
    } catch (std::exception& e) {
        std::cerr << "error: %s\n" << e.what() << "\n";
    }
}

void Listener::shutdown() {
    m_shutdown = true;
}