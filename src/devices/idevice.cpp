#include <mutex>

#include "devices/idevice.hpp"
#include "common/device_result.hpp"

DeviceResult IDevice::safe_execution(const Message &input_msg) {
    std::unique_lock<std::mutex> lock(m_mutex);
    return process_command(input_msg);
}