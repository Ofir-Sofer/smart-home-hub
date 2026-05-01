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

file(GLOB_RECURSE  SOURCES "src/*.cpp")
if (BUILD_TESTS)
    file(GLOB_RECURSE  TESTS "tests/*.cpp")
endif()

add_executable(smart_home_hub main.cpp ${SOURCES} ${TESTS})
target_include_directories(smart_home_hub PRIVATE include)
target_link_libraries(smart_home_hub PRIVATE nlohmann_json::nlohmann_json)