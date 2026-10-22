#include <curl/curl.h>
#include <stdexcept>
#include <string>

#include "smart_home_hub/curl_global_guard.hpp"

CurlGlobalGuard::CurlGlobalGuard() {
    CURLcode curl_code = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (curl_code != CURLE_OK) {
        std::string err_msg = curl_easy_strerror(curl_code);
        throw std::runtime_error("Could not init curl: " + err_msg);
    }
}

CurlGlobalGuard::~CurlGlobalGuard() {
    curl_global_cleanup();
}
