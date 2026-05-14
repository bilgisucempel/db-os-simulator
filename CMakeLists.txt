cmake_minimum_required(VERSION 3.16)
project(db-os-simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/scheduler.cpp
    src/memory.cpp
    src/concurrency.cpp
    src/filesystem.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
