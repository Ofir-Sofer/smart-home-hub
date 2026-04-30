#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>
#include <string>

template <typename T>
class MessageQueue {
public:
    MessageQueue(std::string device_id)
        : m_device_id(device_id) {}

    void push(const T& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push_back(message);
        m_convar.notify_one();
    }

    void push_urgent(const T& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push_front(message);
        m_convar.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_convar.wait(lock, [this] { return !m_queue.empty();});
        T value = m_queue.front();
        m_queue.pop_front();
        return value;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
private:
    std::deque<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_convar;
    std::string m_device_id;
};
