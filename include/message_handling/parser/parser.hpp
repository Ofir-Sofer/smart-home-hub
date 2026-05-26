#pragma once

#include <string>

#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

enum class ParserStatus {
    SUCCESS,
    SHUTDOWN,
    ENCODE_ERROR,
    ROUTING_ERROR
};

class Parser {
public:
    Parser(MessageQueue<std::string>& main_queue, IEncoder& encoder, Server& server)
    :m_main_queue(main_queue), m_encoder(encoder), m_server(server){};
    
    ParserStatus process_message();

private:
    MessageQueue<std::string>& m_main_queue;
    IEncoder& m_encoder;
    Server& m_server;
};
