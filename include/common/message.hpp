#pragma once

#include <string>

enum class Direction {
    TO_DEVICE,
    TO_USER
};

struct Message {
    std::string m_device_id;
    std::string m_cmd;
    std::string m_user_id;
    Direction m_target;
};
