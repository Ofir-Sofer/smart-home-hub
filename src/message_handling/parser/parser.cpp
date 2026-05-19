#include <string>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

void Parser::process_message() {
    std::string user_input = m_user_input_queue.pop();
    Message msg = m_encoder.encode(user_input);
    m_server.push_to_device_queue(msg);
}