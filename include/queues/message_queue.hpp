#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>
#include <string>
#include <optional>
#include <atomic>

template <typename T>
class MessageQueue {
public:
    MessageQueue(const std::string& device_id)
        : m_device_id(device_id) {}

    void push(const T& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_shutdown) return;
        m_queue.push_back(message);
        m_convar.notify_one();
    }

    void push_urgent(const T& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_shutdown) return;
        m_queue.push_front(message);
        m_convar.notify_one();
    }

    [[nodiscard]] std::optional<T> pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_convar.wait(lock, [this] { return !m_queue.empty() || m_shutdown;});
        if (m_shutdown) {
            return std::nullopt;
        }
        T value = m_queue.front();
        m_queue.pop_front();
        return value;
    }

    [[nodiscard]] bool empty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    void shutdown() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_shutdown = true;
        m_queue.clear();
        m_convar.notify_all(); // wake up ALL waiting threads
    }

private:
    std::deque<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_convar;
    std::string m_device_id;
    std::atomic<bool> m_shutdown{false};
};
