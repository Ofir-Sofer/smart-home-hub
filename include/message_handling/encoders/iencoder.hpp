#pragma once

#include <string>

#include "common/message.hpp"

class IEncoder {
public:
    virtual ~IEncoder() = default;

    virtual Message encode(const std::string& input) = 0;
};