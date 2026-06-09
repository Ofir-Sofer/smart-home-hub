#pragma once

#include <string>
#include <cstdint>

enum class Direction {
    TO_DEVICE,
    TO_USER
};

struct Message {
    std::string m_device_id;
    std::string m_cmd;
    int64_t m_user_id;
    Direction m_target;
};

struct RawMessage {
    std::string m_raw_input;
    int64_t m_user_id;
};
