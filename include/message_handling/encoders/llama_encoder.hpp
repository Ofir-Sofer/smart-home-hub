#pragma once

#include <string>
#include <cstdint>

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

    std::string build_prompt(const std::string& user_input) const;
};