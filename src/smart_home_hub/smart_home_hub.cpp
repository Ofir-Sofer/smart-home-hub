#include <iostream>
#include <optional>

#include "smart_home_hub/smart_home_hub.hpp"
#include "smart_home_hub/hub_config.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
#include "message_handling/encoders/llama_encoder.hpp"
#include "listener/feedback_listener.hpp"

namespace {
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

    std::unique_ptr<IEncoder> make_encoder(const HubConfig& hub_config, const DeviceRegistry& device_registry) {
        const EncoderSettings& encoder_settings = hub_config.get_encoder_settings();
        if (encoder_settings.type == "llama") {
            return std::make_unique<LlamaEncoder>(device_registry, encoder_settings.model_path);
        }
        return std::make_unique<SimpleEncoder>();
    }
}

SmartHomeHub::SmartHomeHub()
    : m_temp_dir_guard(m_hub_config.get_temp_path()),
    m_main_queue("main"),
    m_telegram_bot(m_hub_config.get_tg_token()),
    m_voice_manager(m_main_queue, m_telegram_bot),
    m_listener(m_main_queue, m_voice_manager, m_telegram_bot, m_hub_config.get_authorized_users_path()),
    m_device_factory(m_hub_config.get_device_config_path()),
    m_device_registry(m_device_factory),
    m_server(m_device_registry),
    m_encoder(make_encoder(m_hub_config, m_device_registry)),
    m_parser(m_main_queue, *m_encoder, m_server) {}

// shutdown sequence
SmartHomeHub::~SmartHomeHub() {
    m_listener.shutdown();
    if (m_listener_thread.joinable()) {
        m_listener_thread.join();
    }
    m_voice_manager.shutdown();
    if (m_voice_manager_thread.joinable()) {
        m_voice_manager_thread.join();
    }
    m_main_queue.shutdown(); // ensure queue shutdown regardless of shutdown path
    m_device_registry.shutdown_all_queues();
    // device queues are shut down, device threads will exit their loops and can be joined
    for (std::thread& th : m_device_threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    std::cout << "program shutdown\n";
}

void SmartHomeHub::run() {
    std::cout << "Smart Home Hub starting...\n";
    // spawn device threads
    std::vector<std::string> device_id_list = m_device_factory.get_device_id_list();
    for (const std::string& id : device_id_list) {
        m_device_threads.emplace_back(worker, std::ref(m_device_registry), id, std::ref(m_telegram_bot));
    }

    m_voice_manager_thread = std::thread(&VoiceMsgManager::process_recordings, &m_voice_manager);
    m_listener_thread = std::thread(&Listener::start, &m_listener);

    std::cout << "Smart Home Hub ready for input\n";
    // main loop
    while (true) {
        ParserResult res = m_parser.process_message();
        if (res.m_status == ParserStatus::SHUTDOWN) {
            break;
        } else if (res.m_status == ParserStatus::ROUTING_ERROR) {
            m_telegram_bot.getApi().sendMessage(res.m_user_id, "device wasnt found.");
            std::cout << "device wasnt found\n";
        } else if (res.m_status == ParserStatus::ENCODE_ERROR) {
            m_telegram_bot.getApi().sendMessage(res.m_user_id, "there was an issue with user message.");
            std::cout << "there was an issue with user message\n";
        } else {
            std::cout << "message pushed successfully\n";
        }
    }
}
