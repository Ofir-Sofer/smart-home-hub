#pragma once

#include <string>
#include <vector>
#include <thread>
#include <memory>
#include <tgbot/tgbot.h>
#include <curl/curl.h>

//program includes
#include "smart_home_hub/hub_config.hpp"
#include "smart_home_hub/curl_global_guard.hpp"
#include "smart_home_hub/temp_dir_guard.hpp"
#include "queues/message_queue.hpp"
#include "message_handling/voice_msg_manager/voice_msg_manager.hpp"
#include "listener/listener.hpp"
#include "device_management/factory/device_factory.hpp"
#include "device_management/registry/device_registry.hpp"
#include "server/server.hpp"
#include "message_handling/encoders/iencoder.hpp"
#include "message_handling/parser/parser.hpp"
#include "common/message.hpp"

class SmartHomeHub {
public:
    SmartHomeHub();
    ~SmartHomeHub();

    SmartHomeHub(const SmartHomeHub&) = delete;
    SmartHomeHub& operator=(const SmartHomeHub&) = delete;
    SmartHomeHub(SmartHomeHub&&) = delete;
    SmartHomeHub& operator=(SmartHomeHub&&) = delete;

    void run();

private:
    HubConfig m_hub_config;
    CurlGlobalGuard m_curl_global_guard;
    TempDirGuard m_temp_dir_guard;
    MessageQueue<RawMessage> m_main_queue;
    TgBot::Bot m_telegram_bot;
    VoiceMsgManager m_voice_manager;
    Listener m_listener;
    DeviceFactory m_device_factory;
    DeviceRegistry m_device_registry;
    Server m_server;
    std::unique_ptr<IEncoder> m_encoder;
    Parser m_parser;
    std::vector<std::thread> m_device_threads;
    std::thread m_voice_manager_thread;
    std::thread m_listener_thread;
};
