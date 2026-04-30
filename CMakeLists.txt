cmake_minimum_required(VERSION 3.20)
project(smart_home_hub VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(BUILD_TESTS "Build tests" ON)

file(GLOB_RECURSE  SOURCES "src/*.cpp")
if (BUILD_TESTS)
    file(GLOB_RECURSE  TESTS "tests/*.cpp")
endif()

add_executable(smart_home_hub main.cpp ${SOURCES} ${TESTS})
target_include_directories(smart_home_hub PRIVATE include)