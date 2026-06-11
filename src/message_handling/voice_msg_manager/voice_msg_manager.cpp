#include <string>
#include <optional>
#include <cstdint>
#include <ctime>

#include "message_handling/voice_msg_manager/voice_msg_manager.hpp"
#include "queues/message_queue.hpp"
#include "common/message.hpp"

void VoiceMsgManager::process_recordings() {
    while (true) {
        std::optional<RawMessage> wrapped_raw_msg = m_recording_queue.pop();
        if (wrapped_raw_msg.has_value()) {
            RawMessage raw_msg = wrapped_raw_msg.value();
            std::string file_path = raw_msg.m_raw_input;
            int64_t user_id = raw_msg.m_user_id;
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
            if (result == "") {
                m_bot.getApi().sendMessage(user_id, "recording was empty");
            } else {
            raw_msg.m_raw_input = result;
            m_main_queue.push(raw_msg);
            }
            std::remove(file_path.c_str());
            std::remove(target_wav.c_str());
        } else {
            break;
        }
    }
}

void VoiceMsgManager::push(const RawMessage& raw_msg) {
    m_recording_queue.push(raw_msg);
}

void VoiceMsgManager::shutdown() {
    m_recording_queue.shutdown();
}
