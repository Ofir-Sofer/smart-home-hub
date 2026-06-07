#include <string>
#include <optional>
#include <iostream>
#include <ctime>

#include "message_handling/parser/parser.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "server/server.hpp"
#include "common/message.hpp"

ParserStatus Parser::process_message() {
    std::optional<std::string> wrapped_user_input = m_main_queue.pop();
    if (wrapped_user_input.has_value()) {
        std::string user_input = wrapped_user_input.value();
        std::cout << "User sent: " << user_input << "\n";
        if (user_input == "SHUTDOWN!!!") {
            return ParserStatus::SHUTDOWN;
        } else {
            std::string voice_prefix = "voice_msg:";
            size_t prefix_length = voice_prefix.length();
            std::string user_prefix = user_input.substr(0,prefix_length);
            if (user_prefix == voice_prefix) {
                std::string file_path = user_input.substr(prefix_length);
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
                user_input = result;
                std::remove(file_path.c_str());
                std::remove(target_wav.c_str());
            }
        }
        try {
            Message msg = m_encoder.encode(user_input);
            if (m_server.push_to_device_queue(msg) == ServerStatus::SUCCESS) {
                return ParserStatus::SUCCESS;
            } else {
                return ParserStatus::ROUTING_ERROR;
            }
        }
        catch(const std::runtime_error& e) {
            std::cerr << "Encode error: " << e.what() << "\n";
            return ParserStatus::ENCODE_ERROR;
        }
    } else {
        //kill program
        return ParserStatus::SHUTDOWN;
    }
}