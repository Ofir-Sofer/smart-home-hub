#include <string>
#include <filesystem>
#include <system_error>
#include <iostream>

#include "smart_home_hub/temp_dir_guard.hpp"

TempDirGuard::TempDirGuard(const std::string& path)
:m_temp_path(path) {
    std::filesystem::create_directories(m_temp_path);
}

TempDirGuard::~TempDirGuard() {
    std::error_code ec;
    std::filesystem::remove_all(m_temp_path, ec);
    if(ec) {
        std::cerr << "Failed to clean path: " << m_temp_path << ". err message: " << ec.message() << "\n";
    }
}
