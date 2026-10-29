#include <iostream>
#include <csignal>
#include <exception>

#include "smart_home_hub/smart_home_hub.hpp"

int main() {
    std::signal(SIGPIPE, SIG_IGN); //err handle inside device obj that uses socker/pipe
    try {
        SmartHomeHub hub;
        hub.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }


    return 0;
}
