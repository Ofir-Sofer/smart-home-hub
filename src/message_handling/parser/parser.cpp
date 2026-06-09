#include <string>
#include <optional>
#include <iostream>
#include <ctime>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

ParserResult Parser::process_message() {
    std::optional<RawMessage> wrapped_raw_msg = m_main_queue.pop();
    if (wrapped_raw_msg.has_value()) {
        RawMessage raw_msg = wrapped_raw_msg.value();
        std::string user_input = raw_msg.m_raw_input;
        std::cout << "User sent: " << raw_msg.m_raw_input << "\n";
        if (raw_msg.m_raw_input == "SHUTDOWN!!!") {
            ParserResult parse_res = {ParserStatus::SHUTDOWN, raw_msg.m_user_id};
            return parse_res;
        } else {
            std::string voice_prefix = "voice_msg:";
            size_t prefix_length = voice_prefix.length();
            std::string user_prefix = raw_msg.m_raw_input.substr(0,prefix_length);
            if (user_prefix == voice_prefix) {
                std::string file_path = raw_msg.m_raw_input.substr(prefix_length);
                time_t now = time(nullptr);
                std::string target_wav = "/tmp/smart_home_hub/audio/voice_" + std::to_string(now) + ".wav";
                std::string convert_cmd = "ffmpeg -i " + file_path + " " + target_wav + " 2>/dev/null";
                FILE* pipe = popen(convert_cmd.c_str(), "r");
                pclose(pipe);
                std::string speech_to_text_cmd = "python3 scripts/hailo_speech_to_text.py --audio " + target_wav + " 2>/dev/null";
                FILE* stt_pipe = popen(speech_to_text_cmd.c_str(), "r");
                char buffer[256];
                std::string result;
                while (fgets(buffer, sizeof(buffer), stt_pipe)) {
                    result += buffer;
                }
                pclose(stt_pipe);
                result.erase(result.find_last_not_of(" \t\n\r") + 1);
                raw_msg.m_raw_input = result;
                std::remove(file_path.c_str());
                std::remove(target_wav.c_str());
            }
        }
        try {
            Message msg = m_encoder.encode(raw_msg.m_raw_input, raw_msg.m_user_id);
            if (m_server.push_to_device_queue(msg) == ServerStatus::SUCCESS) {
                ParserResult parse_res = {ParserStatus::SUCCESS, raw_msg.m_user_id};
                return parse_res;
            } else {
                ParserResult parse_res = {ParserStatus::ROUTING_ERROR, raw_msg.m_user_id};
                return parse_res;
            }
        }
        catch(const std::runtime_error& e) {
            std::cerr << "Encode error: " << e.what() << "\n";
            ParserResult parse_res = {ParserStatus::ENCODE_ERROR, raw_msg.m_user_id};
            return parse_res;
        }
    } else {
        //if main queue has an empty msg this signals - kill program
        ParserResult parse_res = {ParserStatus::SHUTDOWN, 0};
            return parse_res;
    }
}