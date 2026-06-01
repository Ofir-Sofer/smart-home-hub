#include <string>
#include <optional>
#include <iostream>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

ParserStatus Parser::process_message() {
    std::optional<std::string> wrapped_user_input = m_main_queue.pop();
    if (wrapped_user_input.has_value()) {
        std::string user_input = wrapped_user_input.value();
        if (user_input == "SHUTDOWN!!!") {
            return ParserStatus::SHUTDOWN;
        }
        try {
            Message msg = m_encoder.encode(user_input);
            if (m_server.push_to_device_queue(msg) == ServerStatus::SUCCESS) {
                return ParserStatus::SUCCESS;
            } else {
                return ParserStatus::ROUTING_ERROR;
            }
        }
        catch(const std::runtime_error& e) {
            std::cerr << "Encode error: " << e.what() << "\n";
            return ParserStatus::ENCODE_ERROR;
        }
    } else {
        //kill program
        return ParserStatus::SHUTDOWN;
    }
}