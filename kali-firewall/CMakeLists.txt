cmake_minimum_required(VERSION 3.10)
project(kali-firewall)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Qt5 Packages ---
find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
if(NOT Qt5Widgets_FOUND OR NOT Qt5Network_FOUND)
    message(FATAL_ERROR "Qt5 Widgets and/or Network not found! Please install qtbase5-dev and qtbase5-dev-tools.")
endif()

# --- SQLite3 ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
if(NOT SQLITE3_FOUND)
    message(FATAL_ERROR "sqlite3 not found! Please install libsqlite3-dev.")
endif()

# --- Netfilter Queue ---
pkg_check_modules(NFQUEUE REQUIRED libnetfilter_queue)
if(NOT NFQUEUE_FOUND)
    message(FATAL_ERROR "libnetfilter_queue not found! Please install libnetfilter-queue-dev.")
endif()

# --- nlohmann-json (header-only) ---
find_path(NLOHMANN_JSON_INCLUDE_DIR nlohmann/json.hpp)
if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(FATAL_ERROR "nlohmann/json.hpp not found! Please install nlohmann-json3-dev.")
endif()

# --- Include Directories ---
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/gui
    ${CMAKE_SOURCE_DIR}/process_monitor
    ${NFQUEUE_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${NLOHMANN_JSON_INCLUDE_DIR}
)

# --- Source Files ---
add_executable(firewall
    main.cpp
    core/packet_capture.cpp
    core/rule_engine.cpp
    core/dpi_engine.cpp
    core/traffic_shaper.cpp
    core/logger.cpp
    process_monitor/proc_tracker.cpp
    gui/mainwindow.cpp
    gui/dashboard.cpp
    gui/log_viewer.cpp
    gui/rule_editor.cpp
    gui/traffic_shaper_ui.cpp
    gui/dpi_manager.cpp
)

# --- Link Libraries ---
target_link_libraries(firewall
    Qt5::Widgets
    Qt5::Network
    ${NFQUEUE_LIBRARIES}
    ${SQLITE3_LIBRARIES}
)

# --- Post-Build Messages ---
message(STATUS "Qt5 include dirs: ${Qt5Widgets_INCLUDE_DIRS}")
message(STATUS "NFQUEUE include dirs: ${NFQUEUE_INCLUDE_DIRS}")
message(STATUS "SQLITE3 include dirs: ${SQLITE3_INCLUDE_DIRS}")
message(STATUS "nlohmann-json include dir: ${NLOHMANN_JSON_INCLUDE_DIR}")

message(STATUS "----------------------------------------------------------")
message(STATUS "If you see a FATAL_ERROR above, install missing packages:")
message(STATUS "sudo apt-get install qtbase5-dev qtbase5-dev-tools libsqlite3-dev libnetfilter-queue-dev nlohmann-json3-dev pkg-config")
message(STATUS "----------------------------------------------------------")