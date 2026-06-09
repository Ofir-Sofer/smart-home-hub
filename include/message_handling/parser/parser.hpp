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

struct ParserResult {
    ParserStatus m_status;
    int64_t m_user_id;
};

class Parser {
public:
    Parser(MessageQueue<RawMessage>& main_queue, IEncoder& encoder, Server& server)
    :m_main_queue(main_queue), m_encoder(encoder), m_server(server){};
    
    ParserResult process_message();

private:
    MessageQueue<RawMessage>& m_main_queue;
    IEncoder& m_encoder;
    Server& m_server;
};
