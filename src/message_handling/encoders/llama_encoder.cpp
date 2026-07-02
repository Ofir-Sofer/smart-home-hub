#include <string>
#include <vector>
#include <stdexcept>

#include "message_handling/encoders/llama_encoder.hpp"
#include "message_handling/encoders/smart_encoder.hpp"
#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "llama.h"

namespace { void output_cleanup(std::string& output); }

LlamaEncoder::LlamaEncoder(DeviceRegistry& registry, const std::string& model_path) 
:SmartEncoder(registry){
    llama_model_params model_params = llama_model_default_params();
    llama_log_set([](ggml_log_level level, const char* text, void* user_data) {
        // suppress all llama.cpp logs
    }, nullptr);
    m_model = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!m_model) {
        throw std::runtime_error("Failed to load model: " + model_path);
    }
    llama_context_params ctx_params = llama_context_default_params();
    m_ctx = llama_init_from_model(m_model, ctx_params);
    if (!m_ctx) {
        llama_model_free(m_model);
        throw std::runtime_error("Failed to create llama context");
    }
    m_vocab = llama_model_get_vocab(m_model);
}

LlamaEncoder::~LlamaEncoder() {
    llama_free(m_ctx);
    llama_model_free(m_model);
}

std::string LlamaEncoder::build_prompt(const std::string& user_input) const { 
    std::string prompt = m_per_device_commands_list + ". User said: " + user_input + ". Respond ONLY with device_id:command or \
        device_id:command:value format. Example: mini_inverter:set_mode:cold.\
        If the user's request does not clearly match any device or command, respond with exactly: UNKNOWN";
    prompt = "<|user|>\n" + prompt + "<|end|>\n<|assistant|>\n";
    return prompt;
}

Message LlamaEncoder:: encode(const std::string &input, int64_t user_id) const {
    llama_memory_t mem = llama_get_memory(m_ctx);
    llama_memory_clear(mem, false);
    std::string prompt = build_prompt(input);
    int n_predict = 32;
    //tokenize
    const int n_prompt = -llama_tokenize(m_vocab, prompt.c_str(), prompt.size(), NULL, 0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    if (llama_tokenize(m_vocab, prompt.c_str(), prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
        throw std::runtime_error("error: failed to tokenize the prompt\n");
    }

    // initialize the sampler
    auto sparams = llama_sampler_chain_default_params();
    sparams.no_perf = false;
    llama_sampler * sampler = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(sampler, llama_sampler_init_greedy());

    // prepare a batch for the prompt
    llama_batch batch = llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    if (llama_model_has_encoder(m_model)) {
        if (llama_encode(m_ctx, batch)) {
            throw std::runtime_error("failed to eval\n");
        }
        llama_token decoder_start_token_id = llama_model_decoder_start_token(m_model);
        if (decoder_start_token_id == LLAMA_TOKEN_NULL) {
            decoder_start_token_id = llama_vocab_bos(m_vocab);
        }
        batch = llama_batch_get_one(&decoder_start_token_id, 1);
    }

    // main loop
    int n_decode = 0;
    llama_token new_token_id;
    std::string output;
    for (int n_pos = 0; n_pos + batch.n_tokens < n_prompt + n_predict; ) {
        // evaluate the current batch with the transformer model
        if (llama_decode(m_ctx, batch)) {
            throw std::runtime_error("failed to eval, return code\n");
        }
        n_pos += batch.n_tokens;
        // sample the next token
        {
            new_token_id = llama_sampler_sample(sampler, m_ctx, -1);
            if (llama_vocab_is_eog(m_vocab, new_token_id)) {
                break;
            }
            char buf[128];
            int n = llama_token_to_piece(m_vocab, new_token_id, buf, sizeof(buf), 0, true);
            if (n < 0) {
                throw std::runtime_error("failed to convert token to piece\n");
            }
            std::string piece(buf, n);
            output += piece;
            // prepare the next batch with the sampled token
            batch = llama_batch_get_one(&new_token_id, 1);
            n_decode += 1;
        }
    }
    
    output_cleanup(output);
    
    llama_sampler_free(sampler);
    
    if (output == "UNKNOWN") {
        throw std::runtime_error("No matching device found for input");
    }

    SimpleEncoder simple_encoder;
    return simple_encoder.encode(output, user_id);
}

namespace {
    void output_cleanup(std::string& output) {
        output.erase(0, output.find_first_not_of(" \t\n\r"));
        const std::string assistant_prefix = "<|assistant|>";
        if (output.substr(0, assistant_prefix.size()) == assistant_prefix) {
            output.erase(0, assistant_prefix.size());
        }
        output.erase(0, output.find_first_not_of(" \t\n\r"));
        // keep only first line
        auto newline = output.find('\n');
        if (newline != std::string::npos) {
            output = output.substr(0, newline);
        }
        output.erase(output.find_last_not_of(" \t\n\r") + 1);
    }
}