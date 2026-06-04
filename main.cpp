#include <string>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>
#include <csignal>
#include <nlohmann/json.hpp>
#include <fstream>

//program includes
#include "queues/message_queue.hpp"
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

void worker(DeviceRegistry& device_registry, Server& server, const std::string& device_id) {
    MessageQueue<Message>* device_queue = device_registry.get_queue(device_id);
    IDevice* device = device_registry.get_device(device_id);
    if (!device_queue || !device) {
        std::cerr << "Device not found: " << device_id << "\n";
        return;
    }
    FeedbackListener feedback_listener(server);
    while (true) {
        std::optional<Message> wrapped_msg = device_queue->pop();
        if (!wrapped_msg.has_value()) {
            break;
        }
        DeviceResult res = device->process_command(wrapped_msg.value());
        feedback_listener.forward_to_user(res, wrapped_msg.value().m_user_id, device_id);
    }
}

//handle ctrl+c
std::atomic<bool> g_shutdown{false};
MessageQueue<std::string>* g_main_queue = nullptr;
void signal_handler(int signal) {
    // cleanup code here- change g_shutdown to true
    g_shutdown = true;
    if (g_main_queue) {
        g_main_queue->shutdown();
    }
    std::cout << "\nShutdown signal received\n";
}

int main() {
    std::cout << "Smart Home Hub starting...\n";
    std::signal(SIGINT, signal_handler);
    std::string settings_path = "config/settings.json";
    MessageQueue<std::string> main_queue("main");
    g_main_queue = &main_queue;
    const char* tg_token = std::getenv("TELEGRAM_TOKEN");
    if (!tg_token) {
        std::cerr << "TELEGRAM_TOKEN environment variable not set\n";
        return 1;
    }
    std::string authorized_users_path = "config/authorized_users.json";
    Listener listener(main_queue, tg_token, authorized_users_path);
    std::string device_config_path = "config/devices.json";
    DeviceFactory device_factory(device_config_path);
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    std::unique_ptr<IEncoder> encoder;
    {
        std::ifstream file(settings_path);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open: " + settings_path);
        }
        nlohmann::json j;
        file >> j;
        std::string encoder_type = j["encoder"];
        if (encoder_type == "llama") {
            std::string model_path = j["model_path"];
            encoder = std::make_unique<LlamaEncoder>(device_registry, model_path);
        } else {
            encoder = std::make_unique<SimpleEncoder>();
        }
    }
    Parser parser(main_queue, *encoder, server);

    // spawn device threads
    std::vector<std::thread> device_threads;
    std::vector<std::string> device_id_list = device_factory.get_device_id_list();
    for (const std::string& id : device_id_list) {
        std::thread new_thread(worker,std::ref(device_registry), std::ref(server), id);
        device_threads.push_back(std::move(new_thread));
    }

    // main loop
    std::thread listener_thread([&listener]() {
        listener.start();
    });
    std::cout << "Smart Home Hub ready for input\n";
    while (true) {
        ParserStatus status = parser.process_message();
        if (status == ParserStatus::SHUTDOWN) {
            break;
        } else if (status == ParserStatus::ROUTING_ERROR) {
            std::cout << "device wasnt found\n";
            // TODO use feedback listener to notify user
        } else if (status == ParserStatus::ENCODE_ERROR) {
            std::cout << "there was an issue with user message\n";
            // TODO use feedback listener to notify user
        } else {
            std::cout << "message pushed successfully\n";
        }
    }

    // shutdown sequence
    listener.shutdown();
    if (listener_thread.joinable()) {
        listener_thread.join();
    }
    main_queue.shutdown();// ensure queue shutdown regardless of shutdown path
    device_registry.shutdown_all_queues();
    // device queues are shut down, device threads will exit their loops and can be joined
    for (std::thread& th : device_threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    std::cout << "program shutdown\n";

    return 0;
}