#include <string>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

//program includes
#include "queues/message_queue.hpp"
#include "listener/listener.hpp"
#include "device_management/factory/device_factory.hpp"
#include "device_management/registry/device_registry.hpp"
#include "server/server.hpp"
#include "message_handling/encoders/simple_encoder.hpp"
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

int main() {
    std::cout << "Smart Home Hub starting...\n";
    MessageQueue<std::string> main_queue("main");
    Listener listener(main_queue);
    std::string device_config_path = "config/devices.json";
    DeviceFactory device_factory(device_config_path);
    DeviceRegistry device_registry(device_factory);
    Server server(device_registry);
    SimpleEncoder encoder;
    Parser parser(main_queue, encoder, server);

    // spawn device threads
    std::vector<std::thread> device_threads;
    std::vector<std::string> device_id_list = device_factory.get_device_id_list();
    for (const std::string& id : device_id_list) {
        std::thread new_thread(worker,std::ref(device_registry), std::ref(server), id);
        device_threads.push_back(std::move(new_thread));
    }

    // main loop
    std::string user_input;
    while (std::getline(std::cin, user_input)) {
        if (user_input == "SHUTDOWN!!!") {
            main_queue.shutdown();
            // this causes the main queue to return by default empty msg, which in turn causes the parser to returen shutdown
        } else {
            listener.push_to_main_queue(user_input);
        }
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
    device_registry.shutdown_all_queues();
    // device queues are shut down, device threads will exit their loops and can be joined
    for (std::thread& th : device_threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    return 0;
}