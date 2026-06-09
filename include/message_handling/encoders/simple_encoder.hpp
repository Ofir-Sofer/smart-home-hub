#pragma once

#include <string>
#include <cstdint>

#include "message_handling/encoders/iencoder.hpp"

class SimpleEncoder : public IEncoder {
public:
    Message encode(const std::string& input, int64_t user_id) const override;
};