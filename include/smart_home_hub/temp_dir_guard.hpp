#pragma once

#include <string>

// RAII guard for a temp directory: creates it on construction, recursively deletes it (and everything in it) on destruction.
class TempDirGuard {
public:
    explicit TempDirGuard(const std::string& path);
    ~TempDirGuard();

    TempDirGuard(const TempDirGuard&) = delete;
    TempDirGuard& operator=(const TempDirGuard&) = delete;
    TempDirGuard(TempDirGuard&&) = delete;
    TempDirGuard& operator=(TempDirGuard&&) = delete;
private:
    std::string m_temp_path;
};
