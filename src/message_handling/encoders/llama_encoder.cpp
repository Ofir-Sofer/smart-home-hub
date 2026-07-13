#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <utility>

#include "message_handling/encoders/llama_encoder.hpp"
#include "message_handling/encoders/smart_encoder.hpp"
#include "device_management/registry/device_registry.hpp"
#include "common/message.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "common/command_validation.hpp"
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

std::string LlamaEncoder::build_device_match_prompt(const std::string &user_input) const {
    std::ostringstream oss;
    oss << "Available devices:\n" << m_device_list
        << "\nWhich device_id best matches the user request? Respond with exactly one device_id from the list above, "
        << "using it exactly as written — never invent or modify a device_id."
        << "\nIf the request does not clearly match any device, respond with exactly: UNKNOWN"
        << "\nExamples:"
        << "\nUser said: turn on the ac -> mini_inverter\n"
        << "\nUser said: clean the living room -> roborock\n"
        << "\nUser said: set temperature to 24 -> mini_inverter\n"
        << "\nUser said: run clean program public -> roborock\n"
        << "\nUser said: whats the weather -> UNKNOWN\n"
        << "\nUser said: " << user_input << " -> ";
    return oss.str();
}

std::string LlamaEncoder::build_command_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command_list) const {
    std::ostringstream oss;
    oss << "Device: " << device_id
        << "\nAvailable commands: " << command_list
        << "\nWhich command best matches the user's request? Respond with exactly one command from "
        << "the list above, using it exactly as written — never invent or modify a command name."
        << "\nIf the request does not clearly match any command, respond with exactly: UNKNOWN"
        << "\nExamples:"
        << "\nUser said: start cleaning -> start"
        << "\nUser said: come back to the dock -> return_to_base"
        << "\nUser said: find the vacuum -> locate"
        << "\nUser said: set ac temperature to 24 -> set_temp\n"
        << "\nUser said: run clean program public -> run_routine\n"
        << "\nUser said: " << user_input << "->";
    return oss.str();
}

std::string LlamaEncoder::build_command_ranged_value_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command, const std::pair<int, int> &range) const {
    std::ostringstream oss;
    oss << "Device: " << device_id
        << "\nCommand: " << command
        << "\nAllowed range: " << range.first << " to " << range.second << " (inclusive)"
        << "\nUser said: " << user_input
        << "\nWhat numeric value best matches the user's request? Respond with exactly one integer "
        << "between " <<  range.first << " and " << range.second << "."
        << "\nIf the request does not clearly specify a value in that range, respond with exactly: UNKNOWN"
        << "\nExamples:"
        << "\nUser said: set temperature to 24 -> 24"
        << "\nUser said: make it warmer, like 30 degrees -> 30"
        << "\nUser said: set temperature to 999 -> UNKNOWN";
    return oss.str();
}

std::string LlamaEncoder::build_command_fixed_value_match_prompt(const std::string &user_input, const std::string &device_id, const std::string &command, const std::string &value_list) const {
    std::ostringstream oss;
    oss << "Device: " << device_id
        << "\nCommand: " << command
        << "\nAllowed values: " << value_list
        << "\nUser said: " << user_input
        << "\nWhich value best matches the user's request? Respond with exactly one value from the "
        << "list above, using it exactly as written — never invent or modify a value."
        << "\nThe value must be exactly one of the allowed values listed — if the request does not clearly "
        << "and confidently match one of them, respond with exactly: UNKNOWN"
        << "\nExamples:"
        << "\nUser said: run the public routine -> public"
        << "\nUser said: do a deep clean plus -> deep_plus"
        << "\nUser said: run clean program public -> public\n"
        << "\nUser said: run routine xyzabc -> UNKNOWN";
    return oss.str();
}

Message LlamaEncoder:: encode(const std::string &input, int64_t user_id) const {
    std::string prompt, full_output;
    //stage 1:
    prompt = build_device_match_prompt(input);
    std::string device_id = infer(prompt);
    if (device_id == "UNKNOWN") {
        throw std::runtime_error("No matching device found for input");
    }

    //stage 2:
    std::vector<std::string> allowed_commands = m_registry.get_commands(device_id);
    if (allowed_commands.empty()) {
        throw std::runtime_error("No commands found for device: " + device_id);
    }
    std::string command_list = join_comma_separated(allowed_commands);
    prompt = build_command_match_prompt(input, device_id, command_list);
    std::string command = infer(prompt);
    if (command == "UNKNOWN") {
        throw std::runtime_error("No matching command found for device: " + device_id);
    }
    full_output = device_id + ":" + command;

    //stage 3:
    std::vector<std::string> allowed_values = m_registry.get_command_values(device_id, command);
    int allowed_values_count = allowed_values.size();
    if (allowed_values_count > 0) {
        std::string value;
        if (allowed_values[0].rfind("range(", 0) == 0) {
            std::pair<int, int> range = parse_range(allowed_values[0]);
            prompt = build_command_ranged_value_match_prompt(input, device_id, command, range);
            value = infer(prompt);
            if (value == "UNKNOWN") {
                throw std::runtime_error("No matching value for command " + command + " found for device: " + device_id);
            }
        } else {
            std::string value_list = join_comma_separated(allowed_values);
            prompt = build_command_fixed_value_match_prompt(input, device_id, command, value_list);
            value = infer(prompt);
            if (value == "UNKNOWN") {
                throw std::runtime_error("No matching value for command " + command + " found for device: " + device_id);
            }
        }
        full_output += ":" + value;
    }

    SimpleEncoder simple_encoder;
    return simple_encoder.encode(full_output, user_id);
}

std::string LlamaEncoder::infer(const std::string& prompt) const{
    std::string wrapped_prompt = "<|user|>\n" + prompt + "<|end|>\n<|assistant|>\n";
    llama_memory_t mem = llama_get_memory(m_ctx);
    llama_memory_clear(mem, false);
    int n_predict = 32;
    //tokenize
    const int n_prompt = -llama_tokenize(m_vocab, wrapped_prompt.c_str(), wrapped_prompt.size(), NULL, 0, true, true);
    std::vector<llama_token> prompt_tokens(n_prompt);
    if (llama_tokenize(m_vocab, wrapped_prompt.c_str(), wrapped_prompt.size(), prompt_tokens.data(), prompt_tokens.size(), true, true) < 0) {
        throw std::runtime_error("error: failed to tokenize the prompt\n");
    }

    // initialize the sampler
    auto sparams = llama_sampler_chain_default_params();
    sparams.no_perf = false;
    std::unique_ptr<llama_sampler, decltype(&llama_sampler_free)> sampler(
        llama_sampler_chain_init(sparams), llama_sampler_free);
    llama_sampler_chain_add(sampler.get(), llama_sampler_init_greedy());

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
            new_token_id = llama_sampler_sample(sampler.get(), m_ctx, -1);
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

    return output;
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
