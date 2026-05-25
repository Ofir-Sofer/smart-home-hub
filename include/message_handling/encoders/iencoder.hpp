#pragma once

#include <string>

#include "common/message.hpp"

class IEncoder {
public:
    virtual ~IEncoder() = default;

    [[nodiscard]] virtual Message encode(const std::string& input) const = 0;
};