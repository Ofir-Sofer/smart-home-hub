#include <mutex>

#include "common/shutdown_signal.hpp"

void ShutdownSignal::request() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_shutdown_requested = true;
    m_convar.notify_all();
}

void ShutdownSignal::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_convar.wait(lock, [this] { return m_shutdown_requested;});
}
