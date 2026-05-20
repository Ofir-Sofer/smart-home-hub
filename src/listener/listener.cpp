#include <string>

#include "listener/listener.hpp"

void Listener::push_to_main_queue(const std::string &user_input) {
    m_main_queue.push(user_input);
}
