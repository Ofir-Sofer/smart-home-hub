#pragma once

#include <string>
#include <cstdint>

#include "common/message.hpp"

class IEncoder {
public:
    virtual ~IEncoder() = default;

    [[nodiscard]] virtual Message encode(const std::string& input, int64_t user_id) const = 0;
};