#pragma once

#include "message_handling/encoders/iencoder.hpp"

class SimpleEncoder : public IEncoder {
public:
    Message encode(const std::string& input) const override;
};