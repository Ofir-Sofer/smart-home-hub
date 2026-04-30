#include "message_handling/encoders/simple_encoder.hpp"

#include <string>
#include <sstream>
#include "common/message.hpp"

Message SimpleEncoder::encode(const std::string &input) { 
    size_t split_ind = input.find(':') ;
    
    std::string device = input.substr(0,split_ind);
    std::string cmd = input.substr(split_ind+1);
    std::string user_id = "def_user";
    Direction target = Direction::TO_DEVICE;
    Message msg = {device, cmd, user_id, target};
    return msg;
}