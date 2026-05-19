#pragma once

#include <string>

#include "server/server.hpp"
#include "common/device_result.hpp"

class FeedbackListener {
public:
    FeedbackListener(Server& server)
    :m_server(server){};
    
    void forward_to_user(const DeviceResult& device_res, const std::string& user_id, const std::string& device_id);
private:
    Server& m_server;
};