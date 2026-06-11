#include <string>
#include <optional>
#include <iostream>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

ParserResult Parser::process_message() {
    std::optional<RawMessage> wrapped_raw_msg = m_main_queue.pop();
    if (wrapped_raw_msg.has_value()) {
        RawMessage raw_msg = wrapped_raw_msg.value();
        std::cout << "User sent: " << raw_msg.m_raw_input << "\n";
        if (raw_msg.m_raw_input == "SHUTDOWN!!!") {
            ParserResult parse_res = {ParserStatus::SHUTDOWN, raw_msg.m_user_id};
            return parse_res;
        }
        try {
            Message msg = m_encoder.encode(raw_msg.m_raw_input, raw_msg.m_user_id);
            if (m_server.push_to_device_queue(msg) == ServerStatus::SUCCESS) {
                ParserResult parse_res = {ParserStatus::SUCCESS, raw_msg.m_user_id};
                return parse_res;
            } else {
                ParserResult parse_res = {ParserStatus::ROUTING_ERROR, raw_msg.m_user_id};
                return parse_res;
            }
        }
        catch(const std::runtime_error& e) {
            std::cerr << "Encode error: " << e.what() << "\n";
            ParserResult parse_res = {ParserStatus::ENCODE_ERROR, raw_msg.m_user_id};
            return parse_res;
        }
    } else {
        //if main queue has an empty msg this signals - kill program
        ParserResult parse_res = {ParserStatus::SHUTDOWN, 0};
            return parse_res;
    }
}