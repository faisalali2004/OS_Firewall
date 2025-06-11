cmake_minimum_required(VERSION 3.10)

project(cpp-cli-firewall)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable warnings
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Gather all source files in src/
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Create the executable
add_executable(cpp-cli-firewall ${SOURCES})

# Link with pcap (required for packet capture)
find_package(PCAP REQUIRED)
target_link_libraries(cpp-cli-firewall PRIVATE ${PCAP_LIBRARY})

# For Linux, also link pthread if needed
if(UNIX AND NOT APPLE)
    target_link_libraries(cpp-cli-firewall PRIVATE pthread)
endif()

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)