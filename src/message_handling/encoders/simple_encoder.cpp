#include "message_handling/encoders/simple_encoder.hpp"

#include <string>
#include <stdexcept>
#include <iostream>

#include "common/message.hpp"

Message SimpleEncoder:: encode(const std::string &input, int64_t user_id) const { 
    size_t split_ind = input.find(':');
    if (split_ind == std::string::npos) {
        throw std::runtime_error("message has the wrong format");
    }
    std::string device = input.substr(0,split_ind);
    std::string cmd = input.substr(split_ind+1);
    Direction target = Direction::TO_DEVICE;
    Message msg = {device, cmd, user_id, target};
    return msg;
}