cmake_minimum_required(VERSION 3.16)
project(db-os-simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ortak kaynak dosyalar (her senaryo bunları paylaşır)
set(COMMON_SOURCES
    src/scheduler.cpp
    src/memory.cpp
    src/concurrency.cpp
    src/filesystem.cpp
)

# Senaryo 1: FIFO Scheduler + I/O Blocking + Dosya Cakisma Senaryosu
add_executable(scenario_fifo
    src/scenario_fifo.cpp
    ${COMMON_SOURCES}
)
target_include_directories(scenario_fifo PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Senaryo 2: Round Robin Scheduler Senaryosu
add_executable(scenario_rr
    src/scenario_rr.cpp
    ${COMMON_SOURCES}
)
target_include_directories(scenario_rr PRIVATE ${CMAKE_SOURCE_DIR}/include)
