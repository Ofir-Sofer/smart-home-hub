#pragma once

#include <mutex>
#include <condition_variable>

// One-shot shutdown request: any thread calls request(); the hub's run() blocks in wait() until it does.
class ShutdownSignal {
public:
    ShutdownSignal() = default;
    ~ShutdownSignal() = default;

    ShutdownSignal(const ShutdownSignal&) = delete;
    ShutdownSignal& operator=(const ShutdownSignal&) = delete;
    ShutdownSignal(ShutdownSignal&&) = delete;
    ShutdownSignal& operator=(ShutdownSignal&&) = delete;

    void request();
    void wait();

private:
    std::mutex m_mutex;
    std::condition_variable m_convar;
    bool m_shutdown_requested{false};
};
