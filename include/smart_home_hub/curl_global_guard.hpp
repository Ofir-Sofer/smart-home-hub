#pragma once

// RAII guard for libcurl's global state: init on construction (throws on failure), cleanup on destruction. Must be declared before any curl user.
class CurlGlobalGuard {
public:
    CurlGlobalGuard();
    ~CurlGlobalGuard();

    CurlGlobalGuard(const CurlGlobalGuard&) = delete;
    CurlGlobalGuard& operator=(const CurlGlobalGuard&) = delete;
    CurlGlobalGuard(CurlGlobalGuard&&) = delete;
    CurlGlobalGuard& operator=(CurlGlobalGuard&&) = delete;
};
