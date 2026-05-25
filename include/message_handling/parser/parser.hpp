#pragma once

#include <string>

#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

class Parser {
public:
    Parser(MessageQueue<std::string>& user_input_queue, IEncoder& encoder, Server& server)
    :m_main_queue(user_input_queue), m_encoder(encoder), m_server(server){};
    
    void process_message();

private:
    MessageQueue<std::string>& m_main_queue;
    IEncoder& m_encoder;
    Server& m_server;
};
