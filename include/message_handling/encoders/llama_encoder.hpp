#pragma once

#include <string>

#include "message_handling/encoders/iencoder.hpp"
#include "device_management/registry/device_registry.hpp"
#include "llama.h"

class LlamaEncoder : public IEncoder {
public:
    LlamaEncoder(DeviceRegistry& registry, const std::string& model_path);
    
    ~LlamaEncoder();
    LlamaEncoder(const LlamaEncoder&) = delete;
    LlamaEncoder& operator=(const LlamaEncoder&) = delete;
    LlamaEncoder(LlamaEncoder&&) = delete;
    LlamaEncoder& operator=(LlamaEncoder&&) = delete;

    Message encode(const std::string& input) const override;

private:
    DeviceRegistry& m_registry;
    llama_model* m_model;
    llama_context* m_ctx;
    const llama_vocab* m_vocab;

    std::string build_prompt(const std::string& user_input) const;
};