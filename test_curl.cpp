#include <curl/curl.h>
#include <string>
#include <iostream>


size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* buffer = static_cast<std::string*>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

int main() {
    std::string buffer;
    const char* token = std::getenv("HA_TOKEN");
    if (!token) {
        std::cout << "wrong token\n";
        return 1;
    }
    std::string auth = "Authorization: Bearer " + std::string(token);
    CURL* curl = curl_easy_init();          // 1. create handle
    if(!curl) {
        std::cout << "fail to initialize handle\n";
        return 2;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.68.58:8123/api/services/vacuum/return_to_base");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"entity_id\": \"vacuum.s8_maxv_ultra\"}");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl); 
    if (res != CURLE_OK) {
        std::cout << "curl failed: " << curl_easy_strerror(res) << "\n";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 3;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        std::cout << "HTTP/application error\n";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return 4;
    }
    std::cout << buffer << "\n";

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}