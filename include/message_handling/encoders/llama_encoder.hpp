#pragma once

#include <string>
#include <cstdint>
#include <utility>
#include <vector>

#include "message_handling/encoders/smart_encoder.hpp"
#include "device_management/registry/device_registry.hpp"
#include "llama.h"

class LlamaEncoder : public SmartEncoder {
public:
    LlamaEncoder(DeviceRegistry& registry, const std::string& model_path);

    ~LlamaEncoder();
    LlamaEncoder(const LlamaEncoder&) = delete;
    LlamaEncoder& operator=(const LlamaEncoder&) = delete;
    LlamaEncoder(LlamaEncoder&&) = delete;
    LlamaEncoder& operator=(LlamaEncoder&&) = delete;

    Message encode(const std::string& input, int64_t user_id) const override;

private:
    llama_model* m_model;
    llama_context* m_ctx;
    const llama_vocab* m_vocab;

    std::string build_device_match_prompt(const std::string &user_input) const;
    std::string build_command_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command_list) const;
    std::string build_command_ranged_value_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command, const std::pair<int, int> &range) const;
    std::string build_command_fixed_value_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command, const std::string &value_list) const;
    std::string infer(const std::string& prompt) const;
};
