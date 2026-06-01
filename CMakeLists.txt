cmake_minimum_required(VERSION 3.20)
project(smart_home_hub VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_TESTS "Build tests" ON)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG        v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
    tgbot
    GIT_REPOSITORY https://github.com/reo7sp/tgbot-cpp.git
    GIT_TAG        v1.10
)

FetchContent_MakeAvailable(tgbot)

find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(CURL REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")
if (BUILD_TESTS)
    file(GLOB_RECURSE TESTS "tests/*.cpp")
endif()

# Main application (always built)
add_executable(smart_home_hub main.cpp ${SOURCES})
target_include_directories(smart_home_hub PRIVATE include  ${tgbot_SOURCE_DIR}/include)
target_link_libraries(smart_home_hub PRIVATE 
    nlohmann_json::nlohmann_json 
    TgBot
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    CURL::libcurl
)

# Test runner (only built when BUILD_TESTS=ON)
if(BUILD_TESTS)
    add_executable(smart_home_hub_tests ${SOURCES} ${TESTS})
    target_include_directories(smart_home_hub_tests PRIVATE include ${tgbot_SOURCE_DIR}/include)
    target_link_libraries(smart_home_hub_tests PRIVATE 
    nlohmann_json::nlohmann_json 
    TgBot
    OpenSSL::SSL 
    OpenSSL::Crypto
    ${Boost_LIBRARIES}
    CURL::libcurl
)
endif()