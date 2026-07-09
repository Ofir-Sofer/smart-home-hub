#include <string>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include <csignal>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <tgbot/tgbot.h>
#include <curl/curl.h>

//program includes
#include "queues/message_queue.hpp"
#include "message_handling/voice_msg_manager/voice_msg_manager.hpp"
#include "listener/listener.hpp"
#include "device_management/factory/device_factory.hpp"
#include "device_management/registry/device_registry.hpp"
#include "server/server.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "message_handling/encoders/llama_encoder.hpp"
#include "message_handling/parser/parser.hpp"
#include "common/message.hpp"
#include "listener/feedback_listener.hpp"
#include "devices/idevice.hpp"

void worker(DeviceRegistry& device_registry, const std::string& device_id, TgBot::Bot& bot) {
    MessageQueue<Message>* device_queue = device_registry.get_queue(device_id);
    IDevice* device = device_registry.get_device(device_id);
    if (!device_queue || !device) {
        std::cerr << "Device not found: " << device_id << "\n";
        return;
    }
    FeedbackListener feedback_listener(bot);
    while (true) {
        std::optional<Message> wrapped_msg = device_queue->pop();
        if (!wrapped_msg.has_value()) {
            break;
        }
        DeviceResult res = device->safe_execution(wrapped_msg.value());
        feedback_listener.forward_to_user(res, wrapped_msg.value().m_user_id, device_id, wrapped_msg.value());
    }
}

//handle ctrl+c
std::atomic<bool> g_shutdown{false};
MessageQueue<RawMessage>* g_main_queue = nullptr;
void signal_handler(int signal) {
    // cleanup code here- change g_shutdown to true
    g_shutdown = true;
    if (g_main_queue) {
        g_main_queue->shutdown();
    }
    std::cout << "\nShutdown signal received\n";
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "Smart Home Hub starting...\n";
    std::filesystem::create_directories("/tmp/smart_home_hub/audio");
    std::signal(SIGINT, signal_handler);
    std::signal(SIGPIPE, SIG_IGN); //err handle inside device obj that uses socker/pipe
    std::string settings_path = "config/settings.json";
    MessageQueue<RawMessage> main_queue("main");
    g_main_queue = &main_queue;
    const char* tg_token = std::getenv("TELEGRAM_TOKEN");
    if (!tg_token) {
        std::cerr << "TELEGRAM_TOKEN environment variable not set\n";
        return 1;
    }
    TgBot::Bot telegram_bot(tg_token);
    std::string authorized_users_path = "config/authorized_users.json";
    VoiceMsgManager voice_manager(main_queue, telegram_bot);
    Listener listener(main_queue, voice_manager, telegram_bot, authorized_users_path);
    std::string device_config_path = "config/devices.json";
    DeviceFactory device_factory(device_config_path);
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    std::unique_ptr<IEncoder> encoder;
    try {
        std::ifstream file(settings_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open: " + settings_path);
        }
        nlohmann::json j;
        file >> j;
        std::string encoder_type = j["encoder"];
        if (encoder_type == "llama") {
            std::string active_model = j["active_model"];
            std::string model_path = j["available_models"].at(active_model)["path"];
            encoder = std::make_unique<LlamaEncoder>(device_registry, model_path);
        } else {
            encoder = std::make_unique<SimpleEncoder>();
        }
    } catch (std::exception& e) {
        std::cerr << "settings.json error: " << e.what() << "\n";
        return 1;
    }
    Parser parser(main_queue, *encoder, server);

    // spawn device threads
    std::vector<std::thread> device_threads;
    std::vector<std::string> device_id_list = device_factory.get_device_id_list();
    for (const std::string& id : device_id_list) {
        std::thread new_thread(worker,std::ref(device_registry), id, std::ref(telegram_bot));
        device_threads.push_back(std::move(new_thread));
    }

    // main loop
    std::thread voice_manager_thread([&voice_manager]() {
        voice_manager.process_recordings();
    });
    std::thread listener_thread([&listener]() {
        listener.start();
    });
    std::cout << "Smart Home Hub ready for input\n";
    while (true) {
        ParserResult res = parser.process_message();
        if (res.m_status == ParserStatus::SHUTDOWN) {
            break;
        } else if (res.m_status == ParserStatus::ROUTING_ERROR) {
            telegram_bot.getApi().sendMessage(res.m_user_id, "device wasnt found.");
            std::cout << "device wasnt found\n";
        } else if (res.m_status == ParserStatus::ENCODE_ERROR) {
            telegram_bot.getApi().sendMessage(res.m_user_id, "there was an issue with user message.");
            std::cout << "there was an issue with user message\n";
        } else {
            std::cout << "message pushed successfully\n";
        }
    }

    // shutdown sequence
    listener.shutdown();
    if (listener_thread.joinable()) {
        listener_thread.join();
    }
    voice_manager.shutdown();
    if (voice_manager_thread.joinable()) {
        voice_manager_thread.join();
    }
    main_queue.shutdown();// ensure queue shutdown regardless of shutdown path
    device_registry.shutdown_all_queues();
    // device queues are shut down, device threads will exit their loops and can be joined
    for (std::thread& th : device_threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    std::filesystem::remove_all("/tmp/smart_home_hub");
    curl_global_cleanup(); 
    std::cout << "program shutdown\n";

    return 0;
}