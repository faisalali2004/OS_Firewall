cmake_minimum_required(VERSION 3.16)
project(kali-firewall LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt (change Qt5 to Qt6 if needed)
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)

# Add include directories (root for MOC, core, gui)
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/gui
)

# Gather all source and header files
file(GLOB_RECURSE CORE_SOURCES ${CMAKE_SOURCE_DIR}/core/*.cpp)
file(GLOB_RECURSE GUI_SOURCES ${CMAKE_SOURCE_DIR}/gui/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/core/*.h ${CMAKE_SOURCE_DIR}/gui/*.h)
file(GLOB MAIN_SRC ${CMAKE_SOURCE_DIR}/main.cpp)

add_executable(firewall
    ${MAIN_SRC}
    ${CORE_SOURCES}
    ${GUI_SOURCES}
    ${HEADERS}
)

# Link libraries (Linux: sqlite3, netfilter_queue, nfnetlink)
target_link_libraries(firewall
    Qt5::Widgets
    Qt5::Network
    sqlite3
    netfilter_queue
    nfnetlink
)

# Platform-specific settings
if (WIN32)
    target_compile_definitions(firewall PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Extra warnings and options
if (MSVC)
    target_compile_options(firewall PRIVATE /W4 /permissive-)
else()
    target_compile_options(firewall PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install target (optional)
install(TARGETS firewall DESTINATION bin)