#include <iostream>
#include <csignal>
#include <exception>
#include <pthread.h>
#include <thread>

#include "smart_home_hub/smart_home_hub.hpp"

namespace {
    class SignalWatcher {
    public:
        SignalWatcher(const sigset_t& signals, SmartHomeHub& hub);
        ~SignalWatcher();

        SignalWatcher(const SignalWatcher&) = delete;
        SignalWatcher& operator=(const SignalWatcher&) = delete;
        SignalWatcher(SignalWatcher&&) = delete;
        SignalWatcher& operator=(SignalWatcher&&) = delete;
    private:
        std::thread m_sig_thread;
    };

    SignalWatcher::SignalWatcher(const sigset_t& signals, SmartHomeHub& hub)
        : m_sig_thread([signals, &hub] {
            int sig = 0;
            sigwait(&signals, &sig);
            if (sig != SIGUSR1) {
                hub.request_shutdown();
            }
        })
    {}

    SignalWatcher::~SignalWatcher() {
        if (m_sig_thread.joinable()) {
            pthread_kill(m_sig_thread.native_handle(), SIGUSR1);
            m_sig_thread.join();
        }
    }
}

int main() {
    std::signal(SIGPIPE, SIG_IGN); //err handle inside device obj that uses socket/pipe
    sigset_t signals;
    sigemptyset(&signals);           // start with an empty set
    sigaddset(&signals, SIGINT);     // Ctrl+C
    sigaddset(&signals, SIGTERM);    // kill / systemd stop
    sigaddset(&signals, SIGUSR1);    // our own internal wake-up
    pthread_sigmask(SIG_BLOCK, &signals, nullptr);
    try {
        SmartHomeHub hub;
        SignalWatcher signal_watcher(signals, hub);
        hub.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    return 0;
}
