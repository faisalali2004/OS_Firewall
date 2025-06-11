cmake_minimum_required(VERSION 3.10)

project(cpp-cli-firewall
    VERSION 1.0
    DESCRIPTION "Advanced C++ CLI Firewall"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warn if building in-source
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use a separate build directory:\n"
        "  mkdir build && cd build && cmake ..")
endif()

# Enable warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Source files
file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

# Executable
add_executable(cpp-cli-firewall ${SOURCES})

# Install target
install(TARGETS cpp-cli-firewall DESTINATION bin)

# Optionally, export compile_commands.json for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)